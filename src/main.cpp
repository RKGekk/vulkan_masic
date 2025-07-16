#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <map>
#include <stdexcept>
#include <string>
#include <unordered_set>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;
const char* WINDOW_TITLE = "Vulkan Test";
const char* APP_NAME = "Hello Triangle";
const char* ENGINE_NAME = "No Engine";
const int MAX_FRAMES_IN_FLIGHT = 2;

#ifdef NDEBUG
const bool ENABLE_VALIDATION_LAYERS = false;
#else
const bool ENABLE_VALIDATION_LAYERS = true;
#endif

const std::vector<const char*> VALIDATION_LAYERS = {
    "VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> DEVICE_EXTENSIONS = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
#ifndef NDEBUG
    , VK_EXT_DEBUG_MARKER_EXTENSION_NAME
    //, VK_EXT_DEBUG_UTILS_EXTENSION_NAME
#endif
};

class VulkanApplication;

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
    [[maybe_unused]]
    const VulkanApplication* app = (const VulkanApplication*)pUserData;
    std::cout << "validation layer: " << pCallbackData->pMessage << std::endl;
    
    return VK_FALSE;
}

struct SwapchainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> present_modes;
};

template<typename Container>
void printInfo(std::string_view header, Container container) {
    std::cout << header << ": " << std::endl;
    for(const auto& ext : container) {
        std::cout << "\t - " << ext << std::endl;
    }
}

template<typename C, typename T>
auto insert_in_container(C& c, T&& t) -> decltype(c.push_back(std::forward<T>(t)), void()) {
    c.push_back(std::forward<T>(t));
}
template<typename C, typename T>
auto insert_in_container(C& c, T&& t) -> decltype(c.insert(std::forward<T>(t)), void()) {
    c.insert(std::forward<T>(t));
}

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* p_create_info, const VkAllocationCallbacks* p_allocator, VkDebugUtilsMessengerEXT* p_debug_messenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, p_create_info, p_allocator, p_debug_messenger);
    }
    else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debug_messenger, const VkAllocationCallbacks* p_allocator) {
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) {
		func(instance, debug_messenger, p_allocator);
	}
}

struct QueueFamilyIndices {
    std::optional<uint32_t> graphics_family;
    std::optional<uint32_t> present_family;
    std::optional<uint32_t> transfer_family;
    
    bool isComplete() {
        return graphics_family.has_value() && present_family.has_value() && transfer_family.has_value();
    }
    
    VkSharingMode getBufferSharingMode() {
        if (graphics_family.has_value() && transfer_family.has_value() && graphics_family.value() != transfer_family.value()) {
            return VK_SHARING_MODE_CONCURRENT;
        }
        return VK_SHARING_MODE_EXCLUSIVE;
    }
    
    std::unordered_set<uint32_t> getFamilies() {
        std::unordered_set<uint32_t> family_indices;
        if(graphics_family.has_value()) {
            family_indices.insert(graphics_family.value());
        }
        if(present_family.has_value()) {
            family_indices.insert(present_family.value());
        }
        if(transfer_family.has_value()) {
            family_indices.insert(transfer_family.value());
        }
        return family_indices;
    }
    
    std::vector<uint32_t> getIndices() {
        const auto& families = getFamilies();
        std::vector<uint32_t> result(families.cbegin(), families.cend());
        return result;
    }
};

template<typename Container1, typename Container2>
bool checkNamesSupported(const Container1& available_ext, const Container2& req_layer_names) {
    bool result = false;
    result = std::all_of(req_layer_names.cbegin(), req_layer_names.cend(), [&available_ext](const auto& req_name){ return std::count(available_ext.cbegin(), available_ext.cend(), req_name); });
    return result;
}

std::unordered_set<std::string> getNamesUnsupported(const std::unordered_set<std::string>& available_ext, const std::vector<std::string>& req_layer_names) {
    std::unordered_set<std::string> result;
    std::vector<std::string> from(available_ext.cbegin(), available_ext.cend());
    std::sort(from.begin(), from.end());
    std::vector<std::string> to(req_layer_names.cbegin(), req_layer_names.cend());
    std::sort(to.begin(), to.end());
    std::set_difference(
        from.cbegin(),
        from.cend(),
        to.cbegin(),
        to.cend(),
        std::inserter(result, result.begin())
    );
    return result;
}

struct InstanceExtensions {
    std::unordered_set<std::string> available_instance_ext;
    std::vector<std::string> req_instance_extensions;
    bool is_all_ext_supported = false;
    std::unordered_set<std::string> diff_instance_extensions;

    template<typename Container>
    static Container getAvailableInstanceExtensions() {
        uint32_t ext_count = 0u;
        VkResult result = vkEnumerateInstanceExtensionProperties(nullptr, &ext_count, nullptr);
        if(result != VK_SUCCESS) {
            throw std::runtime_error("failed to read instance properties!");
        }
        std::vector<VkExtensionProperties> ext_props(ext_count);
        result = vkEnumerateInstanceExtensionProperties(nullptr, &ext_count, ext_props.data());
        if(result != VK_SUCCESS) {
            throw std::runtime_error("failed to read instance properties!");
        }
        Container available_ext;
        available_ext.reserve(ext_count);
        for(const VkExtensionProperties& prop : ext_props) {
            insert_in_container(available_ext, std::string(prop.extensionName));
        }
        
        return available_ext;
    }

    template<typename Container>
    static Container getRequiredInstanceExtensions() {
		uint32_t glfw_extension_count = 0;
		const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);
        Container extensions(glfw_extensions, glfw_extensions + glfw_extension_count);
		if (ENABLE_VALIDATION_LAYERS) {
			insert_in_container(extensions, VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}
#ifdef __APPLE__
        insert_in_container(extensions, VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
#endif
#ifndef NDEBUG
        insert_in_container(extensions, VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
#endif
		return extensions;
	}

    std::vector<const char*> get_ppNames() const {
        std::vector<const char*> req_instance_extensions_ppNames;
        req_instance_extensions_ppNames.reserve(req_instance_extensions.size());
        std::transform(req_instance_extensions.cbegin(), req_instance_extensions.cend(), std::inserter(req_instance_extensions_ppNames, req_instance_extensions_ppNames.end()), [&](const std::string& str) { return str.c_str(); });
        return req_instance_extensions_ppNames;
    }

    void init() {
        available_instance_ext = getAvailableInstanceExtensions<std::unordered_set<std::string>>();
        std::vector<std::string> req_instance_extensions = getRequiredInstanceExtensions<std::vector<std::string>>();
        is_all_ext_supported = checkNamesSupported(available_instance_ext, req_instance_extensions);
        if(!is_all_ext_supported) {
            diff_instance_extensions = getNamesUnsupported(available_instance_ext, req_instance_extensions);
        }
    }
};

struct InstanceLayers {
    std::unordered_set<std::string> available_validation_layers;
    std::vector<std::string> req_validation_layers;
    bool is_all_layers_supported = false;
    std::unordered_set<std::string> diff_validation_layers;

    template<typename Container>
    static Container getAvailableValidationLayers() {
        uint32_t layers_count = 0u;
        VkResult result = vkEnumerateInstanceLayerProperties(&layers_count, nullptr);
        if(result != VK_SUCCESS) {
            throw std::runtime_error("failed to read validation layer properties!");
        }
        std::vector<VkLayerProperties> layer_props(layers_count);
        result = vkEnumerateInstanceLayerProperties(&layers_count, layer_props.data());
        if(result != VK_SUCCESS) {
            throw std::runtime_error("failed to read validation layer properties!");
        }
        Container validation_layers;
        validation_layers.reserve(layers_count);
        for(const VkLayerProperties& prop : layer_props) {
            insert_in_container(validation_layers, std::string(prop.layerName));
        }
        
        return validation_layers;
    }

    template<typename Container>
    static Container getRequiredValidationLayers() {
		Container layers;
		if (ENABLE_VALIDATION_LAYERS) {
			layers.insert(layers.begin(), VALIDATION_LAYERS.cbegin(), VALIDATION_LAYERS.cend());
		}
		return layers;
	}

    std::vector<const char*> get_ppNames() const {
        std::vector<const char*> req_instance_layer_ppNames;
        req_instance_layer_ppNames.reserve(req_validation_layers.size());
        std::transform(req_validation_layers.cbegin(), req_validation_layers.cend(), std::inserter(req_instance_layer_ppNames, req_instance_layer_ppNames.begin()), [&](const std::string& str) { return str.c_str(); });
        return req_instance_layer_ppNames;
    }

    void init() {
        available_validation_layers = getAvailableValidationLayers<std::unordered_set<std::string>>();
        req_validation_layers = getRequiredValidationLayers<std::vector<std::string>>();
        bool is_all_layers_supported = checkNamesSupported(available_validation_layers, req_validation_layers);
        if(!is_all_layers_supported) {
            diff_validation_layers = getNamesUnsupported(available_validation_layers, req_validation_layers);
        }
    }
};

struct DeviceExtensions {
    std::unordered_set<std::string> available_device_ext;
    std::vector<std::string> req_device_ext;
    bool all_device_ext_supported;

    template<typename Container>
    static Container getDeviceExtensionSupported(VkPhysicalDevice physical_device) {
        uint32_t extensions_count = 0u;
        VkResult result = vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extensions_count, nullptr);
        if(result != VK_SUCCESS) {
            throw std::runtime_error("failed to read instance properties!");
        }
        std::vector<VkExtensionProperties> available_extensions(extensions_count);
        result = vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extensions_count, available_extensions.data());
        if(result != VK_SUCCESS) {
            throw std::runtime_error("failed to read instance properties!");
        }
    
        Container available_ext;
        available_ext.reserve(extensions_count);
        for(const VkExtensionProperties& prop : available_extensions) {
            insert_in_container(available_ext, std::string(prop.extensionName));
        }
        
        return available_ext;
    }

    template<typename Container>
    Container getRequiredDeviceExtensions() {
        Container extensions {"VK_KHR_portability_subset"};
        extensions.insert(extensions.begin(), DEVICE_EXTENSIONS.cbegin(), DEVICE_EXTENSIONS.cend());
        return extensions;
    }

    void init(VkPhysicalDevice physical_device) {
        available_device_ext = getDeviceExtensionSupported<std::unordered_set<std::string>>(physical_device);
        req_device_ext = getRequiredDeviceExtensions<std::vector<std::string>>();
        all_device_ext_supported = checkNamesSupported(available_device_ext, req_device_ext);
    }
};

class VulkanApplication {
public:
    void run() {
        glfwInit();
        
        m_window = initMainWindow();
        initVulkan(m_window);
        mainLoop();
        cleanup();

        glfwTerminate();
    }

private:
    GLFWwindow* m_window = nullptr;
    VkInstance m_vk_instance = VK_NULL_HANDLE;
    InstanceExtensions m_instance_extensions;
    InstanceLayers m_validation_layers;
    VkDebugUtilsMessengerEXT m_debug_messenger = VK_NULL_HANDLE;
    VkPhysicalDevice m_physical_device = VK_NULL_HANDLE;
    DeviceExtensions m_device_extensions;
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;

    GLFWwindow* initMainWindow() {
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        GLFWwindow* result = glfwCreateWindow(WIDTH, HEIGHT, WINDOW_TITLE, nullptr, nullptr);

        return result;
    }

    uint32_t getVkApiVersion() {
        uint32_t api_version = VK_API_VERSION_1_0;
        VkResult result = vkEnumerateInstanceVersion(&api_version);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to get Vulkan API version!");
        }
        return api_version;
    }

    VkInstance createInstance(const InstanceExtensions& inst_ext, const InstanceLayers& inst_layers) {
        VkApplicationInfo app_info{};
        app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        app_info.pApplicationName = APP_NAME;
        app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        app_info.pEngineName = ENGINE_NAME;
        app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        app_info.apiVersion = getVkApiVersion();

        VkInstanceCreateInfo instance_info{};
        instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instance_info.pApplicationInfo = &app_info;
        
#ifndef NDEBUG
        printInfo("Supported instance extensions", inst_ext.available_instance_ext);
#endif
        if(!inst_ext.is_all_ext_supported) {
            for(const std::string& name : inst_ext.diff_instance_extensions) {
                throw std::runtime_error("instance not support extension - " + name);
            }
        }
        
        std::vector<const char*> req_instance_extensions_ppNames = inst_ext.get_ppNames();
        instance_info.ppEnabledExtensionNames = req_instance_extensions_ppNames.data();
        instance_info.enabledExtensionCount = static_cast<uint32_t>(req_instance_extensions_ppNames.size());

#ifndef NDEBUG
        printInfo("Supported validation layers", inst_layers.available_validation_layers);
        
        if(!inst_layers.is_all_layers_supported) {
            for(const std::string& name : inst_layers.diff_validation_layers) {
                throw std::runtime_error("instance not support layer - " + name);
            }
        }
        
        const std::vector<const char*> req_instance_layer_ppNames = inst_layers.get_ppNames();
        instance_info.ppEnabledLayerNames = req_instance_layer_ppNames.data();
        instance_info.enabledLayerCount = static_cast<uint32_t>(req_instance_layer_ppNames.size());
        
        VkDebugUtilsMessengerCreateInfoEXT messenger_info{};
        populateDebugMessengerCreateInfo(messenger_info);
        instance_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &messenger_info;
#else
        instance_info.enabledLayerCount = 0u;
#endif
        
#ifdef __APPLE__
        instance_info.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif
        
        VkInstance instance = VK_NULL_HANDLE;
        VkResult result = vkCreateInstance(&instance_info, nullptr, &instance);
        if(result != VK_SUCCESS) {
            throw std::runtime_error("failed to create instance!");
        }
        
        return instance;
    }

    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& messenger_info) {
        messenger_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        messenger_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        messenger_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        messenger_info.pfnUserCallback = debugCallback;
        messenger_info.pUserData = (void*)this;
    }

    VkDebugUtilsMessengerEXT setupDebugMessanger() {
        if(!ENABLE_VALIDATION_LAYERS) {
            return VK_NULL_HANDLE;
        }
        
        VkDebugUtilsMessengerCreateInfoEXT messenger_info{};
        populateDebugMessengerCreateInfo(messenger_info);
        
        VkDebugUtilsMessengerEXT debug_messanger = VK_NULL_HANDLE;
        VkResult result = CreateDebugUtilsMessengerEXT(m_vk_instance, &messenger_info, nullptr, &debug_messanger);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to set up debug messenger!");
        }
        
        return debug_messanger;
    }

    std::vector<VkPhysicalDevice> getPhysicalDevices(VkInstance vk_instance) {
        uint32_t devices_count = 0u;
        VkResult result = vkEnumeratePhysicalDevices(vk_instance, &devices_count, nullptr);
        if(result != VK_SUCCESS) {
            throw std::runtime_error("failed to enumerate devices!");
        }
        if(!devices_count) {
            throw std::runtime_error("failed to find GPUs with Vulkan support!");
        }
        
        std::vector<VkPhysicalDevice> devices(devices_count);
        result = vkEnumeratePhysicalDevices(vk_instance, &devices_count, devices.data());
        if(result != VK_SUCCESS) {
            throw std::runtime_error("failed to enumerate devices!");
        }
        
        return devices;
    }

    uint64_t getDeviceMaxMemoryLimit(VkPhysicalDevice device) {
        uint64_t max_memory_limit = 0;
        VkPhysicalDeviceMemoryProperties phys_device_mem_prop;
        vkGetPhysicalDeviceMemoryProperties(device, &phys_device_mem_prop);
        for (uint32_t i = 0u; i < phys_device_mem_prop.memoryHeapCount; ++i) {
            VkMemoryHeap mem_heap_prop = phys_device_mem_prop.memoryHeaps[i];
            max_memory_limit += mem_heap_prop.size;
        }
        return max_memory_limit;
    }

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) {
        QueueFamilyIndices indices;
        
        uint32_t queue_family_count = 0u;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);
        std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families.data());
        
        for (int i = 0; const auto& queue_family : queue_families) {
            if(queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                indices.graphics_family = i;
            }
            if(queue_family.queueFlags & VK_QUEUE_TRANSFER_BIT) {
                indices.transfer_family = i;
            }
            VkBool32 present_support = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &present_support);
            if(present_support) {
                indices.present_family = i;
            }
            
            if (indices.isComplete()) {
                break;
            }
            ++i;
        }
        
        return indices;
    }

    SwapchainSupportDetails querySwapChainSupport(VkPhysicalDevice device) {
        SwapchainSupportDetails details;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_surface, &details.capabilities);
        
        uint32_t format_count = 0u;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &format_count, nullptr);
        if(format_count) {
            details.formats.resize(format_count);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &format_count, details.formats.data());
        }
        
        uint32_t present_mode_count = 0u;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &present_mode_count, nullptr);
        if(present_mode_count) {
            details.present_modes.resize(present_mode_count);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &present_mode_count, details.present_modes.data());
        }
        
        return details;
    }

    bool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface) {
        VkPhysicalDeviceFeatures supported_features{};
        vkGetPhysicalDeviceFeatures(device, &supported_features);
    
        QueueFamilyIndices queue_family_indices = findQueueFamilies(device, surface);
        
        DeviceExtensions dev_ext;
        dev_ext.init(device);

        bool all_queue_families_supported = queue_family_indices.isComplete();
        
        bool swap_chain_adequate = false;
        if(dev_ext.all_device_ext_supported) {
            SwapchainSupportDetails swap_chain_details = querySwapChainSupport(device);
            swap_chain_adequate = !swap_chain_details.formats.empty() && !swap_chain_details.present_modes.empty();
        }
        
        bool result = all_queue_families_supported && dev_ext.all_device_ext_supported && swap_chain_adequate && supported_features.samplerAnisotropy;
        return result;
    }

    int rateDeviceSuitability(VkPhysicalDevice device) {
        VkPhysicalDeviceProperties device_props;
        vkGetPhysicalDeviceProperties(device, &device_props);
        
        VkPhysicalDeviceFeatures device_features;
        vkGetPhysicalDeviceFeatures(device, &device_features);
        
        int score = 0;
        
        if(device_props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            score += 1000;
        }
        score += device_props.limits.maxImageDimension2D;
        
        uint64_t max_memory = getDeviceMaxMemoryLimit(device);
        score += (int)std::log2(max_memory);
        
        if(!device_features.geometryShader) {
            score += 1000;
        }
        if(isDeviceSuitable(device, m_surface)){
            return 0;
        }
        
        return score;
    }

    VkPhysicalDevice pickPhysicalDevice(VkInstance vk_instance) {        
        std::vector<VkPhysicalDevice> devices = getPhysicalDevices(vk_instance);
        
        std::multimap<int, VkPhysicalDevice> candidates;
        for(const auto& device : devices){
            int score = rateDeviceSuitability(device);
            candidates.insert(std::make_pair(score, device));
        }
        
        VkPhysicalDevice physical_device = VK_NULL_HANDLE;
        if (candidates.size() > 0u) {
            physical_device = candidates.rbegin()->second;
        }
        else {
            throw std::runtime_error("failed to find a suitable GPU!");
        }
        
        return physical_device;
    }

    VkSurfaceKHR createSurface(VkInstance vk_instance, GLFWwindow* glfw_window_ptr) {
        VkSurfaceKHR surface = VK_NULL_HANDLE;
        VkResult result = glfwCreateWindowSurface(vk_instance, glfw_window_ptr, nullptr, &surface);
        if(result != VK_SUCCESS) {
            throw std::runtime_error("failed to create window surface!");
        }
        return surface;
    }

    void initVulkan(GLFWwindow* glfw_window_ptr) {
        m_instance_extensions.init();
        m_validation_layers.init();
        m_vk_instance = createInstance(m_instance_extensions, m_validation_layers);
        m_debug_messenger = setupDebugMessanger();
        m_surface = createSurface(m_vk_instance, glfw_window_ptr);
        m_physical_device = pickPhysicalDevice(m_vk_instance);
        m_device_extensions.init(m_physical_device);
#ifndef NDEBUG
        printInfo("Supported device extensions", m_device_extensions.available_device_ext);
#endif
    }

    void mainLoop() {
        while(!glfwWindowShouldClose(m_window)) {
            glfwPollEvents();
        }
    }

    void cleanup() {
        if (ENABLE_VALIDATION_LAYERS) {
            DestroyDebugUtilsMessengerEXT(m_vk_instance, m_debug_messenger, nullptr);
        }
        vkDestroyInstance(m_vk_instance, nullptr);
        glfwDestroyWindow(m_window);
    }
};

int main(int, char**){
    VulkanApplication app;

    try {
        app.run();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    
    return 0;
}