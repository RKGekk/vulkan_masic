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
#include <array>
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
#include <optional>
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
    //, VK_EXT_DEBUG_MARKER_EXTENSION_NAME
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

struct SwapchainParams {
    VkSurfaceFormatKHR surface_format;
    VkPresentModeKHR present_mode;
    VkExtent2D extent;
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
    std::optional<uint32_t> compute_family;
    std::optional<uint32_t> transfer_family;

    void init(VkPhysicalDevice device, VkSurfaceKHR surface) {
        uint32_t queue_family_count = 0u;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);
        
        std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families.data());
        
        int i = 0;
        for (const VkQueueFamilyProperties& queue_family : queue_families) {
            if(queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                graphics_family = i;
            }
            if(queue_family.queueFlags & VK_QUEUE_TRANSFER_BIT) {
                transfer_family = i;
            }
            if(queue_family.queueFlags & VK_QUEUE_COMPUTE_BIT) {
                compute_family = i;
            }
            VkBool32 present_support = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &present_support);
            if(present_support) {
                present_family = i;
            }
            
            if (isComplete()) {
                break;
            }
            ++i;
        }
    }
    
    bool isComplete() const {
        return graphics_family.has_value() && present_family.has_value() && compute_family.has_value() && transfer_family.has_value();
    }
    
    VkSharingMode getBufferSharingMode() const {
        if (graphics_family.has_value() && transfer_family.has_value() && graphics_family.value() != transfer_family.value()) {
            return VK_SHARING_MODE_CONCURRENT;
        }
        return VK_SHARING_MODE_EXCLUSIVE;
    }
    
    std::unordered_set<uint32_t> getFamilies() const {
        std::unordered_set<uint32_t> family_indices;
        if(graphics_family.has_value()) {
            family_indices.insert(graphics_family.value());
        }
        if(present_family.has_value()) {
            family_indices.insert(present_family.value());
        }
        if(compute_family.has_value()) {
            family_indices.insert(compute_family.value());
        }
        if(transfer_family.has_value()) {
            family_indices.insert(transfer_family.value());
        }
        return family_indices;
    }
    
    std::vector<uint32_t> getIndices() const {
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
        req_instance_extensions = getRequiredInstanceExtensions<std::vector<std::string>>();
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
        is_all_layers_supported = checkNamesSupported(available_validation_layers, req_validation_layers);
        if(!is_all_layers_supported) {
            diff_validation_layers = getNamesUnsupported(available_validation_layers, req_validation_layers);
        }
    }
};

struct DeviceExtensions {
    std::unordered_set<std::string> available_device_ext;
    std::vector<std::string> req_device_ext;
    bool all_device_ext_supported;
    std::unordered_set<std::string> diff_device_ext;

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
        Container extensions;
#ifdef __APPLE__
        extensions.insert("VK_KHR_portability_subset");
#endif
        extensions.insert(extensions.begin(), DEVICE_EXTENSIONS.cbegin(), DEVICE_EXTENSIONS.cend());
        return extensions;
    }

    std::vector<const char*> get_ppNames() const {
        std::vector<const char*> req_device_extensions_ppNames;
        req_device_extensions_ppNames.reserve(req_device_ext.size());
        std::transform(req_device_ext.cbegin(), req_device_ext.cend(), std::inserter(req_device_extensions_ppNames, req_device_extensions_ppNames.begin()), [&](const std::string& str) { return str.c_str(); });
        return req_device_extensions_ppNames;
    }

    void init(VkPhysicalDevice physical_device) {
        available_device_ext = getDeviceExtensionSupported<std::unordered_set<std::string>>(physical_device);
        req_device_ext = getRequiredDeviceExtensions<std::vector<std::string>>();
        all_device_ext_supported = checkNamesSupported(available_device_ext, req_device_ext);
        if(!all_device_ext_supported) {
            diff_device_ext = getNamesUnsupported(available_device_ext, req_device_ext);
        }
    }
};

class InputFileStramGuard final {
public:
    InputFileStramGuard(std::ifstream&& stream) : m_stream(std::move(stream)) {}
    ~InputFileStramGuard() {
        m_stream.close();
    }
    InputFileStramGuard(const InputFileStramGuard&) = delete;
    InputFileStramGuard& operator=(const InputFileStramGuard&) = delete;
    
    std::ifstream& Get() {
        return m_stream;
    }
private:
    std::ifstream m_stream;
};

struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 tex_coord;
    
    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription binding_desc{};
        binding_desc.binding = 0u;
        binding_desc.stride = sizeof(Vertex);
        binding_desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        
        return binding_desc;
    }
    
    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescritpions() {
        std::array<VkVertexInputAttributeDescription, 3> attribute_desc{};
        attribute_desc[0].binding = 0;
        attribute_desc[0].location = 0;
        attribute_desc[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attribute_desc[0].offset = offsetof(Vertex, pos);
        
        attribute_desc[1].binding = 0;
        attribute_desc[1].location = 1;
        attribute_desc[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attribute_desc[1].offset = offsetof(Vertex, color);
        
        attribute_desc[2].binding = 0;
        attribute_desc[2].location = 2;
        attribute_desc[2].format = VK_FORMAT_R32G32_SFLOAT;
        attribute_desc[2].offset = offsetof(Vertex, tex_coord);
        
        return attribute_desc;
    }
};

const std::vector<Vertex> g_vertices = {
    {{-0.5f, -0.5f,  0.0f}, { 1.0f,  0.0f,  0.0f}, { 0.0f,  0.0f}},
    {{ 0.5f, -0.5f,  0.0f}, { 0.0f,  1.0f,  0.0f}, { 1.0f,  0.0f}},
    {{ 0.5f,  0.5f,  0.0f}, { 0.0f,  0.0f,  1.0f}, { 1.0f,  1.0f}},
    {{-0.5f,  0.5f,  0.0f}, { 1.0f,  1.0f,  1.0f}, { 0.0f,  1.0f}},

    {{-0.5f, -0.5f, -0.5f}, { 1.0f,  0.0f,  0.0f}, { 0.0f,  0.0f}},
    {{ 0.5f, -0.5f, -0.5f}, { 0.0f,  1.0f,  0.0f}, { 1.0f,  0.0f}},
    {{ 0.5f,  0.5f, -0.5f}, { 0.0f,  0.0f,  1.0f}, { 1.0f,  1.0f}},
    {{-0.5f,  0.5f, -0.5f}, { 1.0f,  1.0f,  1.0f}, { 0.0f,  1.0f}}
};

const std::vector<uint16_t> g_indices = {
    0, 1, 2,
    2, 3, 0,
    4, 5, 6,
    6, 7, 4
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
    VkDevice m_device = VK_NULL_HANDLE;
    QueueFamilyIndices m_queue_family_indices;
    VkQueue m_graphics_queue = VK_NULL_HANDLE;
    VkQueue m_compute_queue = VK_NULL_HANDLE;
    VkQueue m_transfer_queue = VK_NULL_HANDLE;
    VkQueue m_present_queue = VK_NULL_HANDLE;
    SwapchainSupportDetails m_swapchain_support_details;
    SwapchainParams m_swapchain_params;
    VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
    std::vector<VkImage> m_swapchain_images;
    std::vector<VkImageView> m_swapchain_views;
    std::vector<VkFramebuffer> m_swapchain_framebuffers;
    VkImage m_depth_image = VK_NULL_HANDLE;
    VkDeviceMemory m_depth_memory = VK_NULL_HANDLE;
    VkImageView m_depth_view = VK_NULL_HANDLE;
    VkSampleCountFlagBits m_msaa_samples = VK_SAMPLE_COUNT_1_BIT;
    VkImage m_color_image;
    VkDeviceMemory m_color_image_memory;
    VkImageView m_color_image_view;
    VkRenderPass m_render_pass = VK_NULL_HANDLE;
    VkCommandPool m_grapics_cmd_pool = VK_NULL_HANDLE;
    VkCommandPool m_transfer_cmd_pool = VK_NULL_HANDLE;
    VkCommandPool m_compute_cmd_pool = VK_NULL_HANDLE;
    VkShaderModule m_vert_shader_modeule = VK_NULL_HANDLE;
    VkShaderModule m_frag_shader_modeule = VK_NULL_HANDLE;
    VkDescriptorSetLayout m_desc_set_layout = VK_NULL_HANDLE;
    VkPipelineLayout m_pipeline_layout = VK_NULL_HANDLE;
    VkPipeline m_graphics_pipeline = VK_NULL_HANDLE;

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

    SwapchainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface) {
        SwapchainSupportDetails details;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);
        
        uint32_t format_count = 0u;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, nullptr);
        if(format_count) {
            details.formats.resize(format_count);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, details.formats.data());
        }
        
        uint32_t present_mode_count = 0u;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count, nullptr);
        if(present_mode_count) {
            details.present_modes.resize(present_mode_count);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count, details.present_modes.data());
        }
        
        return details;
    }

    bool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface) {
        VkPhysicalDeviceFeatures supported_features{};
        vkGetPhysicalDeviceFeatures(device, &supported_features);
    
        QueueFamilyIndices queue_family_indices;
        queue_family_indices.init(device, surface);
        
        DeviceExtensions dev_ext;
        dev_ext.init(device);

        bool all_queue_families_supported = queue_family_indices.isComplete();
        
        bool swap_chain_adequate = false;
        if(dev_ext.all_device_ext_supported) {
            SwapchainSupportDetails swap_chain_details = querySwapChainSupport(device, surface);
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
        
        if(device_features.geometryShader) {
            score += 1000;
        }
        if(!isDeviceSuitable(device, m_surface)){
            return 0;
        }
        
        return score;
    }

    VkPhysicalDevice pickPhysicalDevice(VkInstance vk_instance) {        
        std::vector<VkPhysicalDevice> devices = getPhysicalDevices(vk_instance);
        
        std::multimap<int, VkPhysicalDevice> candidates;
        for(VkPhysicalDevice device : devices){
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

    VkDevice createLogicalDevice(VkPhysicalDevice physical_device, const QueueFamilyIndices& queue_family_indices, const DeviceExtensions& device_extensions, const InstanceLayers& validation_layers) {
        std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
        const std::unordered_set<uint32_t>& family_indices = queue_family_indices.getFamilies();
        float queue_priority = 1.0f;
        for(uint32_t family_index : family_indices) {
            VkDeviceQueueCreateInfo queue_create_info{};
            queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queue_create_info.queueFamilyIndex = family_index;
            queue_create_info.queueCount = 1u;
            queue_create_info.pQueuePriorities = &queue_priority;
            queue_create_infos.push_back(queue_create_info);
        }

        VkPhysicalDeviceFeatures device_features{};
        device_features.samplerAnisotropy = VK_TRUE;

        VkDeviceCreateInfo device_create_info{};
        device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        device_create_info.pQueueCreateInfos = queue_create_infos.data();
        device_create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());
        device_create_info.pEnabledFeatures = &device_features;

        std::vector<const char*> device_ext = device_extensions.get_ppNames();
        device_create_info.enabledExtensionCount = static_cast<uint32_t>(device_ext.size());
        device_create_info.ppEnabledExtensionNames = device_ext.data();

        std::vector<const char*> validation_layers_ppNames = validation_layers.get_ppNames();
        if(ENABLE_VALIDATION_LAYERS) {
            device_create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers_ppNames.size());
            device_create_info.ppEnabledLayerNames = validation_layers_ppNames.data();
        }
        else {
            device_create_info.enabledLayerCount = 0u;
        }

        VkDevice device;
        VkResult result = vkCreateDevice(physical_device, &device_create_info, nullptr, &device);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to create logical device!");
        }
        
        return device;
    }

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& available_formats) {
        for(const auto& available_format : available_formats) {
            if(available_format.format == VK_FORMAT_B8G8R8A8_SRGB && available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return available_format;
            }
        }
        return available_formats[0];
    }

    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& available_present_modes){
        for (const auto& available_present_mode : available_present_modes) {
            if(available_present_mode == VK_PRESENT_MODE_MAILBOX_KHR) {
                return available_present_mode;
            }
        }
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
        if(capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return capabilities.currentExtent;
        }
        else {
            int width = 0;
            int height = 0;
            glfwGetFramebufferSize(m_window, &width, &height);
            
            VkExtent2D actual_extent = {
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height)
            };
            
            actual_extent.width = std::clamp(actual_extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            actual_extent.height = std::clamp(actual_extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
            
            return actual_extent;
        }
    }

    VkSwapchainKHR createSwapchain(VkDevice logical_device, VkSurfaceKHR surface, const SwapchainParams& swapchain_params, const SwapchainSupportDetails& swapchain_support_details, const QueueFamilyIndices& queue_family_indices) {
        uint32_t image_count = swapchain_support_details.capabilities.minImageCount + 1u;
        if(swapchain_support_details.capabilities.maxImageCount > 0u && image_count > swapchain_support_details.capabilities.maxImageCount){
            image_count = swapchain_support_details.capabilities.maxImageCount;
        }
        
        VkSwapchainCreateInfoKHR swapchain_create_info{};
        swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swapchain_create_info.surface = surface;
        swapchain_create_info.minImageCount = image_count;
        swapchain_create_info.imageFormat = swapchain_params.surface_format.format;
        swapchain_create_info.imageColorSpace = swapchain_params.surface_format.colorSpace;
        swapchain_create_info.imageExtent = swapchain_params.extent;
        swapchain_create_info.imageArrayLayers = 1u;
        swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        
        std::vector<uint32_t> family_indices = queue_family_indices.getIndices();
        if(queue_family_indices.graphics_family != queue_family_indices.present_family) {
            swapchain_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            swapchain_create_info.queueFamilyIndexCount = static_cast<uint32_t>(family_indices.size());
            swapchain_create_info.pQueueFamilyIndices = family_indices.data();
        }
        else {
            swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            swapchain_create_info.queueFamilyIndexCount = 0u;
            swapchain_create_info.pQueueFamilyIndices = nullptr;
        }
        
        swapchain_create_info.preTransform = swapchain_support_details.capabilities.currentTransform;
        swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        swapchain_create_info.presentMode = swapchain_params.present_mode;
        swapchain_create_info.clipped = VK_TRUE;
        swapchain_create_info.oldSwapchain = VK_NULL_HANDLE;
        
        VkSwapchainKHR swap_chain = VK_NULL_HANDLE;
        VkResult result = vkCreateSwapchainKHR(logical_device, &swapchain_create_info, nullptr, &swap_chain);
        if(result != VK_SUCCESS) {
            throw std::runtime_error("failed to create swap chain!");
        }
        
        return swap_chain;
    }

    std::vector<VkImage> getSwapchainImages(VkDevice device, VkSwapchainKHR swapchain) {
        uint32_t swapchain_image_count = 0u;
        VkResult result = vkGetSwapchainImagesKHR(device, swapchain, &swapchain_image_count, nullptr);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to get swapchain images!");
        }
        std::vector<VkImage> swapchain_images(swapchain_image_count);
        result = vkGetSwapchainImagesKHR(device, swapchain, &swapchain_image_count, swapchain_images.data());
        if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to get swapchain images!");
        }
        
        return swapchain_images; 
    }

    uint32_t findMemoryType(uint32_t type_filter, VkMemoryPropertyFlags properties) {
        VkPhysicalDeviceMemoryProperties mem_prop{};
        vkGetPhysicalDeviceMemoryProperties(m_physical_device, &mem_prop);
        for(uint32_t i = 0u; i < mem_prop.memoryTypeCount; ++i) {
            bool is_type_suit = type_filter & (1 << i);
            bool is_type_adequate = mem_prop.memoryTypes[i].propertyFlags & properties;
            if(is_type_suit && is_type_adequate) {
                return i;
            }
        }
                
        throw std::runtime_error("failed to find suitable memory type!");
    }

    void createImage(VkDevice device, const VkImageCreateInfo& image_info, VkImage& image, VkDeviceMemory& memory, VkMemoryPropertyFlags properties) {
        VkResult result = vkCreateImage(device, &image_info, nullptr, &image);
        if(result != VK_SUCCESS) {
            throw std::runtime_error("failed to create image!");
        }
        
        VkMemoryRequirements mem_req{};
        vkGetImageMemoryRequirements(device, image, &mem_req);
        
        uint32_t mem_type_idx = findMemoryType(mem_req.memoryTypeBits, properties);
        VkMemoryAllocateInfo alloc_info{};
        alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        alloc_info.allocationSize = mem_req.size;
        alloc_info.memoryTypeIndex = mem_type_idx;
        
        result = vkAllocateMemory(device, &alloc_info, nullptr, &memory);
        if(result != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate image memory!");
        }
        vkBindImageMemory(device, image, memory, 0u);
    }

    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspect_flags, uint32_t mip_levels) {
        VkImageViewCreateInfo view_info{};
        view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        view_info.image = image;
        view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        view_info.format = format;
        view_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        view_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        view_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        view_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        view_info.subresourceRange.aspectMask = aspect_flags;
        view_info.subresourceRange.baseMipLevel = 0u;
        view_info.subresourceRange.levelCount = mip_levels;
        view_info.subresourceRange.baseMipLevel = 0u;
        view_info.subresourceRange.layerCount = 1u;
        
        VkImageView image_view;
        VkResult result = vkCreateImageView(m_device, &view_info, nullptr, &image_view);
        if(result != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture image view!");
        }
        
        return image_view;
    }

    void createColorResources(VkDevice device) {
        VkFormat color_format = m_swapchain_params.surface_format.format;
        VkImageCreateInfo image_info{};
        image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        image_info.imageType = VK_IMAGE_TYPE_2D;
        image_info.extent.width = static_cast<uint32_t>(m_swapchain_params.extent.width);
        image_info.extent.height = static_cast<uint32_t>(m_swapchain_params.extent.height);
        image_info.extent.depth = 1u;
        image_info.mipLevels = 1u;
        image_info.arrayLayers = 1u;
        image_info.format = color_format;
        image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
        image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        image_info.usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        image_info.samples = m_msaa_samples;
        image_info.flags = 0u;
        createImage(device, image_info, m_color_image, m_color_image_memory, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        m_color_image_view = createImageView(m_color_image, color_format, VK_IMAGE_ASPECT_COLOR_BIT, 1);
    }

    VkImageView createImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspect_flags, uint32_t mip_levels) {
        VkImageViewCreateInfo view_info{};
        view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        view_info.image = image;
        view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        view_info.format = format;
        view_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        view_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        view_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        view_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        view_info.subresourceRange.aspectMask = aspect_flags;
        view_info.subresourceRange.baseMipLevel = 0u;
        view_info.subresourceRange.levelCount = mip_levels;
        view_info.subresourceRange.baseMipLevel = 0u;
        view_info.subresourceRange.layerCount = 1u;
        
        VkImageView image_view;
        VkResult result = vkCreateImageView(device, &view_info, nullptr, &image_view);
        if(result != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture image view!");
        }
        
        return image_view;
    }

    std::vector<VkImageView> getImageViews(VkDevice device, const std::vector<VkImage>& images, VkSurfaceFormatKHR surface_format) {
        uint32_t sz = static_cast<uint32_t>(images.size());
        std::vector<VkImageView> image_views(sz);
        for(uint32_t i = 0u; i < sz; ++i) {
            image_views[i] = createImageView(device, images[i], surface_format.format, VK_IMAGE_ASPECT_COLOR_BIT, 1u);
        }
        
        return image_views;
    }

    void createSwapchain() {
        m_swapchain_support_details = querySwapChainSupport(m_physical_device, m_surface);
        
        m_swapchain_params.surface_format = chooseSwapSurfaceFormat(m_swapchain_support_details.formats);
        m_swapchain_params.present_mode = chooseSwapPresentMode(m_swapchain_support_details.present_modes);
        m_swapchain_params.extent = chooseSwapExtent(m_swapchain_support_details.capabilities);
        
        m_swapchain = createSwapchain(m_device, m_surface, m_swapchain_params, m_swapchain_support_details, m_queue_family_indices);
        m_swapchain_images = getSwapchainImages(m_device, m_swapchain);
        
        m_swapchain_views = getImageViews(m_device, m_swapchain_images, m_swapchain_params.surface_format);
    }

    std::vector<VkFramebuffer> createFramebuffers(VkDevice device, const std::vector<VkImageView>& views, VkImageView color_image_view, VkImageView depth_view, VkExtent2D extent, VkRenderPass render_pass) {
        size_t ct = views.size();
        std::vector<VkFramebuffer> result_framebuffers(ct);
        for(size_t i = 0u; i < ct; ++i) {
            std::array<VkImageView, 3> attachments = {color_image_view, depth_view, views[i]};
            VkFramebufferCreateInfo framebuffer_info{};
            framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebuffer_info.renderPass = render_pass;
            framebuffer_info.attachmentCount = static_cast<uint32_t>(attachments.size());
            framebuffer_info.pAttachments = attachments.data();
            framebuffer_info.width = extent.width;
            framebuffer_info.height = extent.height;
            framebuffer_info.layers = 1u;
            
            VkResult result = vkCreateFramebuffer(device, &framebuffer_info, nullptr, &result_framebuffers[i]);
            if(result != VK_SUCCESS) {
                throw std::runtime_error("failed to create framebuffer!");
            }
        }
        
        return result_framebuffers;
    }

    VkCommandBuffer beginSingleTimeCommands(VkDevice device, VkCommandPool command_pool) {
        VkCommandBufferAllocateInfo alloc_info{};
        alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        alloc_info.commandPool = command_pool;
        alloc_info.commandBufferCount = 1u;
        
        VkCommandBuffer command_buffer;
        vkAllocateCommandBuffers(device, &alloc_info, &command_buffer);
        
        VkCommandBufferBeginInfo begin_info{};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(command_buffer, &begin_info);
        
        return command_buffer;
    }

    void endSingleTimeCommands(VkDevice device, VkCommandBuffer command_buffer, VkQueue queue, VkCommandPool command_pool) {
        vkEndCommandBuffer(command_buffer);
        
        VkSubmitInfo submit_info{};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.commandBufferCount = 1u;
        submit_info.pCommandBuffers = &command_buffer;
        
        vkQueueSubmit(queue, 1u, &submit_info, VK_NULL_HANDLE);
        vkQueueWaitIdle(queue);
        vkFreeCommandBuffers(device, command_pool, 1u, &command_buffer);
    }

    bool hasStencilComponent(VkFormat format) {
        return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
    }

    void transitionImageLayout(VkDevice device, VkCommandPool cmd_pool, VkQueue queue, VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout, uint32_t mip_levels) {
        VkCommandBuffer command_buffer = beginSingleTimeCommands(device, cmd_pool);
        
        VkPipelineStageFlags source_stage;
        VkPipelineStageFlags destination_stage;
        
        VkImageMemoryBarrier barrier{};
        if (new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            if (hasStencilComponent(format)) {
                barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
            }
        }
        else {
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        }
        
        if(old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            barrier.srcAccessMask = 0u;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        else if(old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        else if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
            barrier.srcAccessMask = 0u;
            barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destination_stage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        }
        else {
            throw std::invalid_argument("unsupported layout transition!");
        }
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = old_layout;
        barrier.newLayout = new_layout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        //barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0u;
        barrier.subresourceRange.levelCount = mip_levels;
        barrier.subresourceRange.baseArrayLayer = 0u;
        barrier.subresourceRange.layerCount = 1u;
        vkCmdPipelineBarrier(command_buffer, source_stage, destination_stage, 0u, 0u, nullptr, 0u, nullptr, 1u, &barrier);
        
        endSingleTimeCommands(device, command_buffer, queue, cmd_pool);
    }

    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
        for (VkFormat format : candidates) {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(m_physical_device, format, &props);
            if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
                return format;
            }
            else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
                return format;
            }
        }
        throw std::runtime_error("failed to find supported format!");
    }

    VkFormat findDepthFormat() {
        return findSupportedFormat(
            {
                VK_FORMAT_D32_SFLOAT,
                VK_FORMAT_D32_SFLOAT_S8_UINT,
                VK_FORMAT_D24_UNORM_S8_UINT
            },
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
        );
    }

    void createDepthResources(VkDevice device, VkCommandPool cmd_pool, VkQueue queue) {
        VkFormat depth_format = findDepthFormat();
        
        VkImageCreateInfo image_info{};
        image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        image_info.imageType = VK_IMAGE_TYPE_2D;
        image_info.extent.width = static_cast<uint32_t>(m_swapchain_params.extent.width);
        image_info.extent.height = static_cast<uint32_t>(m_swapchain_params.extent.height);
        image_info.extent.depth = 1u;
        image_info.mipLevels = 1u;
        image_info.arrayLayers = 1u;
        image_info.format = depth_format;
        image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
        image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        image_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        image_info.samples = m_msaa_samples;
        image_info.flags = 0u;
        createImage(device, image_info, m_depth_image, m_depth_memory, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        m_depth_view = createImageView(m_depth_image, depth_format, VK_IMAGE_ASPECT_DEPTH_BIT, 1u);
        transitionImageLayout(device, cmd_pool, queue, m_depth_image, depth_format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1u);
    }

    VkSampleCountFlagBits getMaxUsableSampleCount(VkPhysicalDevice physical_device) {
        VkPhysicalDeviceProperties physical_device_properties;
        vkGetPhysicalDeviceProperties(physical_device, &physical_device_properties);
        VkSampleCountFlags counts = physical_device_properties.limits.framebufferColorSampleCounts & physical_device_properties.limits.framebufferDepthSampleCounts;
        if (counts & VK_SAMPLE_COUNT_64_BIT) {
            return VK_SAMPLE_COUNT_64_BIT;
        }
        if (counts & VK_SAMPLE_COUNT_32_BIT) {
            return VK_SAMPLE_COUNT_32_BIT;
        }
        if (counts & VK_SAMPLE_COUNT_16_BIT) {
            return VK_SAMPLE_COUNT_16_BIT;
        }
        if (counts & VK_SAMPLE_COUNT_8_BIT) {
            return VK_SAMPLE_COUNT_8_BIT;
        }
        if (counts & VK_SAMPLE_COUNT_4_BIT) {
            return VK_SAMPLE_COUNT_4_BIT;
        }
        if (counts & VK_SAMPLE_COUNT_2_BIT) {
            return VK_SAMPLE_COUNT_2_BIT;
        }
        return VK_SAMPLE_COUNT_1_BIT;
    }

    void createCommandPools(VkDevice device, const QueueFamilyIndices& queue_family_indices) {
        VkCommandPoolCreateInfo gfx_cmd_pool_info{};
        gfx_cmd_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        gfx_cmd_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        gfx_cmd_pool_info.queueFamilyIndex = queue_family_indices.graphics_family.value();
        
        VkResult result = vkCreateCommandPool(device, &gfx_cmd_pool_info, nullptr, &m_grapics_cmd_pool);
        if(result != VK_SUCCESS) {
            throw std::runtime_error("failed to create command pool!");
        }
        
        VkCommandPoolCreateInfo transfer_cmd_pool_info{};
        transfer_cmd_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        transfer_cmd_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
        transfer_cmd_pool_info.queueFamilyIndex = queue_family_indices.transfer_family.value();
        
        result = vkCreateCommandPool(device, &transfer_cmd_pool_info, nullptr, &m_transfer_cmd_pool);
        if(result != VK_SUCCESS) {
            throw std::runtime_error("failed to create command pool!");
        }
    }

    VkRenderPass createRenderPass(VkDevice device, const SwapchainParams& swapchain_params, VkSampleCountFlagBits msaa_samples, VkSubpassDependency pass_dependency) {
        VkAttachmentDescription color_attachment{};
        color_attachment.format = swapchain_params.surface_format.format;
        color_attachment.samples = msaa_samples;
        color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        color_attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        
        VkAttachmentReference color_attachment_ref{};
        color_attachment_ref.attachment = 0u;
        color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        
        VkAttachmentDescription depth_attachment{};
        depth_attachment.format = findDepthFormat();
        depth_attachment.samples = msaa_samples;
        depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        
        VkAttachmentReference depth_attachment_ref{};
        depth_attachment_ref.attachment = 1;
        depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        
        VkAttachmentDescription color_attachment_resolve{};
        color_attachment_resolve.format = swapchain_params.surface_format.format;
        color_attachment_resolve.samples = VK_SAMPLE_COUNT_1_BIT;
        color_attachment_resolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        color_attachment_resolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        color_attachment_resolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        color_attachment_resolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        color_attachment_resolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        color_attachment_resolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        
        VkAttachmentReference color_attachment_resolve_ref{};
        color_attachment_resolve_ref.attachment = 2;
        color_attachment_resolve_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        
        VkSubpassDescription subpass_desc{};
        subpass_desc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass_desc.colorAttachmentCount = 1u;
        subpass_desc.pColorAttachments = &color_attachment_ref;
        subpass_desc.pDepthStencilAttachment = &depth_attachment_ref;
        subpass_desc.pResolveAttachments = &color_attachment_resolve_ref;
        
        std::array<VkAttachmentDescription, 3> attachments = {color_attachment, depth_attachment, color_attachment_resolve};
        
        VkRenderPassCreateInfo render_pass_info{};
        render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        render_pass_info.attachmentCount = static_cast<uint32_t>(attachments.size());
        render_pass_info.pAttachments = attachments.data();
        render_pass_info.subpassCount = 1u;
        render_pass_info.pSubpasses = &subpass_desc;
        render_pass_info.dependencyCount = 1u;
        render_pass_info.pDependencies = &pass_dependency;
        
        VkRenderPass render_pass = VK_NULL_HANDLE;
        VkResult result = vkCreateRenderPass(device, &render_pass_info, nullptr, &render_pass);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to create render pass!");
        }
        
        return render_pass;
    }

    static std::vector<char> readFile(const std::string& file_name) {
    
        InputFileStramGuard stream_guard(std::ifstream(file_name, std::ios::ate | std::ios::binary));
        std::ifstream& file = stream_guard.Get(); 
        if(!file.is_open()) {
            throw std::runtime_error("failed to open file: " + file_name + "\n");
        }

        size_t file_size = (size_t)file.tellg();
        std::vector<char> buffer(file_size);

        file.seekg(0u);
        file.read(buffer.data(), file_size);

        return buffer;
    }

    VkShaderModule CreateShaderModule(const std::vector<char>& buffer) {
        VkShaderModuleCreateInfo shader_module_info{};
        shader_module_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        shader_module_info.codeSize = buffer.size();
        shader_module_info.pCode = reinterpret_cast<const uint32_t*>(buffer.data());
        VkShaderModule shader_module = VK_NULL_HANDLE;
        VkResult result = vkCreateShaderModule(m_device, &shader_module_info, nullptr, &shader_module);
        if(result != VK_SUCCESS) {
            throw std::runtime_error("failed to create shader!");
        }
        return shader_module;
    }

    VkShaderModule CreateShaderModule(const std::string& path) {
        auto shader_buff = readFile(path);
        VkShaderModule shader_modeule = CreateShaderModule(shader_buff);
        return shader_modeule;
    }

    void loadShaders() {
        m_frag_shader_modeule = CreateShaderModule("shaders/shader.frag.spv");    
        m_vert_shader_modeule = CreateShaderModule("shaders/shader.vert.spv");
    }

    VkDescriptorSetLayout createDescSetLayout() {
        VkDescriptorSetLayoutBinding ubo_layout_binding{};
        ubo_layout_binding.binding = 0u;
        ubo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        ubo_layout_binding.descriptorCount = 1u;
        ubo_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        ubo_layout_binding.pImmutableSamplers = nullptr;
        
        VkDescriptorSetLayoutBinding sampler_layout_binding{};
        sampler_layout_binding.binding = 1u;
        sampler_layout_binding.descriptorCount = 1u;
        sampler_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        sampler_layout_binding.pImmutableSamplers = nullptr;
        sampler_layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        
        std::array<VkDescriptorSetLayoutBinding, 2> bindings = {ubo_layout_binding, sampler_layout_binding};
     
        VkDescriptorSetLayoutCreateInfo layout_info{};
        layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layout_info.bindingCount = static_cast<uint32_t>(bindings.size());
        layout_info.pBindings = bindings.data();
        
        VkDescriptorSetLayout desc_set_layout = VK_NULL_HANDLE;
        VkResult result = vkCreateDescriptorSetLayout(m_device, &layout_info, nullptr, &desc_set_layout);
        if(result != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
        
        return desc_set_layout;
    }

    void createPipeline(VkDevice device, VkShaderModule vert_shader_modeule, VkShaderModule frag_shader_modeule, VkRenderPass render_pass, SwapchainParams swapchain_params, VkDescriptorSetLayout desc_set_layout, VkSampleCountFlagBits msaa_samples) {
        VkPipelineShaderStageCreateInfo frag_shader_info{};
        frag_shader_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        frag_shader_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        frag_shader_info.module = frag_shader_modeule;
        frag_shader_info.pName = "main";
        frag_shader_info.pSpecializationInfo = nullptr; 
    
        VkPipelineShaderStageCreateInfo vertex_shader_info{};
        vertex_shader_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertex_shader_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertex_shader_info.module = vert_shader_modeule;
        vertex_shader_info.pName = "main";
        vertex_shader_info.pSpecializationInfo = nullptr;
        VkPipelineShaderStageCreateInfo shader_stages[] = {frag_shader_info, vertex_shader_info};
        
        std::array<VkDynamicState, 2> dynamic_states = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };
        
        VkPipelineDynamicStateCreateInfo dynamic_state_info{};
        dynamic_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamic_state_info.dynamicStateCount = static_cast<uint32_t>(dynamic_states.size());
        dynamic_state_info.pDynamicStates = dynamic_states.data();
        
        auto binding_desc = Vertex::getBindingDescription();
        auto attribute_desc = Vertex::getAttributeDescritpions();
        
        VkPipelineDepthStencilStateCreateInfo depth_stencil_info{};
        depth_stencil_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depth_stencil_info.depthTestEnable = VK_TRUE;
        depth_stencil_info.depthWriteEnable = VK_TRUE;
        depth_stencil_info.depthCompareOp = VK_COMPARE_OP_LESS;
        depth_stencil_info.depthBoundsTestEnable = VK_FALSE;
        depth_stencil_info.minDepthBounds = 0.0f;
        depth_stencil_info.maxDepthBounds = 1.0f;
        depth_stencil_info.stencilTestEnable = VK_FALSE;
        depth_stencil_info.front = {};
        depth_stencil_info.back = {};
        
        VkPipelineVertexInputStateCreateInfo vertex_input_info{};
        vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertex_input_info.vertexBindingDescriptionCount = 1u;
        vertex_input_info.pVertexBindingDescriptions = &binding_desc;
        vertex_input_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(attribute_desc.size());
        vertex_input_info.pVertexAttributeDescriptions = attribute_desc.data();
        
        VkPipelineInputAssemblyStateCreateInfo input_assembly_info{};
        input_assembly_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        input_assembly_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        input_assembly_info.primitiveRestartEnable = VK_FALSE;
        
        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = swapchain_params.extent;
        
        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float)swapchain_params.extent.width;
        viewport.height = (float)swapchain_params.extent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        
        VkPipelineViewportStateCreateInfo viewport_state_info{};
        viewport_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewport_state_info.viewportCount = 1u;
        viewport_state_info.pViewports = &viewport;
        viewport_state_info.scissorCount = 1u;
        viewport_state_info.pScissors = &scissor;
        
        VkPipelineRasterizationStateCreateInfo rasterizer_info{};
        rasterizer_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer_info.depthBiasClamp = VK_FALSE;
        rasterizer_info.rasterizerDiscardEnable = VK_FALSE;
        rasterizer_info.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer_info.cullMode = VK_CULL_MODE_BACK_BIT;
        //rasterizer_info.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterizer_info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizer_info.depthBiasEnable = VK_FALSE;
        rasterizer_info.depthBiasConstantFactor = 0.0f;
        rasterizer_info.depthBiasClamp = 0.0f;
        rasterizer_info.depthBiasSlopeFactor = 0.0f;
        rasterizer_info.lineWidth = 1.0f;
        
        VkPipelineMultisampleStateCreateInfo multisample_info{};
        multisample_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisample_info.sampleShadingEnable = VK_FALSE;
        multisample_info.rasterizationSamples = msaa_samples;
        //multisample_info.minSampleShading = 1.0f;
        //multisample_info.pSampleMask = nullptr;
        //multisample_info.alphaToCoverageEnable = VK_FALSE;
        //multisample_info.alphaToOneEnable = VK_FALSE;
        
        VkPipelineColorBlendAttachmentState color_blend_state{};
        color_blend_state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        color_blend_state.blendEnable = VK_FALSE;
        color_blend_state.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
        color_blend_state.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
        color_blend_state.colorBlendOp = VK_BLEND_OP_ADD;
        color_blend_state.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        color_blend_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        color_blend_state.alphaBlendOp = VK_BLEND_OP_ADD;
        
        VkPipelineColorBlendStateCreateInfo color_blend_info{};
        color_blend_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        color_blend_info.logicOpEnable = VK_FALSE;
        color_blend_info.logicOp = VK_LOGIC_OP_COPY;
        color_blend_info.attachmentCount = 1u;
        color_blend_info.pAttachments = &color_blend_state;
        color_blend_info.blendConstants[0] = 0.0f;
        color_blend_info.blendConstants[1] = 0.0f;
        color_blend_info.blendConstants[2] = 0.0f;
        color_blend_info.blendConstants[3] = 0.0f;
         
        VkPipelineLayoutCreateInfo pipeline_layout_info{};
        pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipeline_layout_info.setLayoutCount = 1u;
        pipeline_layout_info.pSetLayouts = &desc_set_layout;
        pipeline_layout_info.pushConstantRangeCount = 0u;
        pipeline_layout_info.pPushConstantRanges = nullptr;
        
        VkResult result = vkCreatePipelineLayout(device, &pipeline_layout_info, nullptr, &m_pipeline_layout);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }
        
        VkGraphicsPipelineCreateInfo pipeline_info{};
        pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipeline_info.stageCount = 2;
        pipeline_info.pStages = shader_stages;
        pipeline_info.pVertexInputState = &vertex_input_info;
        pipeline_info.pInputAssemblyState = &input_assembly_info;
        pipeline_info.pViewportState = &viewport_state_info;
        pipeline_info.pRasterizationState = &rasterizer_info;
        pipeline_info.pMultisampleState = &multisample_info;
        pipeline_info.pDepthStencilState = &depth_stencil_info;
        pipeline_info.pColorBlendState = &color_blend_info;
        pipeline_info.pDynamicState = &dynamic_state_info;
        pipeline_info.layout = m_pipeline_layout;
        pipeline_info.renderPass = render_pass;
        pipeline_info.subpass = 0u;
        pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
        pipeline_info.basePipelineIndex = -1;
        
        result = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &m_graphics_pipeline);
        
        if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to create graphics pipeline!");
        }
    }

    void createRenderPass() {
        VkSubpassDependency pass_dependency{};
        pass_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        pass_dependency.dstSubpass = 0;
        pass_dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        pass_dependency.srcAccessMask = 0u;
        pass_dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        pass_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        m_render_pass = createRenderPass(m_device, m_swapchain_params, m_msaa_samples, pass_dependency);
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
        m_msaa_samples = getMaxUsableSampleCount(m_physical_device);
        m_queue_family_indices.init(m_physical_device, m_surface);
        m_device = createLogicalDevice(m_physical_device, m_queue_family_indices, m_device_extensions, m_validation_layers);

        vkGetDeviceQueue(m_device, m_queue_family_indices.graphics_family.value(), 0u, &m_graphics_queue);
        vkGetDeviceQueue(m_device, m_queue_family_indices.present_family.value(), 0u, &m_present_queue);
        vkGetDeviceQueue(m_device, m_queue_family_indices.compute_family.value(), 0u, &m_compute_queue);
        vkGetDeviceQueue(m_device, m_queue_family_indices.transfer_family.value(), 0u, &m_transfer_queue);

        createSwapchain();
        loadShaders();
        createRenderPass();
        m_desc_set_layout = createDescSetLayout();
        m_pipeline_layout = VK_NULL_HANDLE;
        createPipeline(m_device, m_vert_shader_modeule, m_frag_shader_modeule, m_render_pass, m_swapchain_params, m_desc_set_layout, m_msaa_samples);
        createCommandPools(m_device, m_queue_family_indices);
        createColorResources(m_device);
        createDepthResources(m_device, m_grapics_cmd_pool, m_graphics_queue);
        m_swapchain_framebuffers = createFramebuffers(m_device, m_swapchain_views, m_color_image_view, m_depth_view, m_swapchain_params.extent, m_render_pass);
    }

    void mainLoop() {
        while(!glfwWindowShouldClose(m_window)) {
            glfwPollEvents();
        }
    }

    void cleanupSwapchain() {
        vkDestroyImageView(m_device, m_color_image_view, nullptr);
        vkDestroyImage(m_device, m_color_image, nullptr);
        vkFreeMemory(m_device, m_color_image_memory, nullptr);
    
        vkDestroyImageView(m_device, m_depth_view, nullptr);
        vkDestroyImage(m_device, m_depth_image, nullptr);
        vkFreeMemory(m_device, m_depth_memory, nullptr);
    
        size_t sz = m_swapchain_framebuffers.size();
        for(size_t i = 0u; i < sz; ++i) {
            vkDestroyFramebuffer(m_device, m_swapchain_framebuffers[i], nullptr);
        }
        for(size_t i = 0u; i < sz; ++i) {
            vkDestroyImageView(m_device, m_swapchain_views[i], nullptr);
        }
        vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
    }

    void cleanup() {
        cleanupSwapchain();
        vkDestroyRenderPass(m_device, m_render_pass, nullptr);
        if(m_grapics_cmd_pool != m_transfer_cmd_pool) {
            vkDestroyCommandPool(m_device, m_grapics_cmd_pool, nullptr);
            vkDestroyCommandPool(m_device, m_transfer_cmd_pool, nullptr);
        }
        else {
            vkDestroyCommandPool(m_device, m_grapics_cmd_pool, nullptr);
        }
        vkDestroyDevice(m_device, nullptr);
        if (ENABLE_VALIDATION_LAYERS) {
            DestroyDebugUtilsMessengerEXT(m_vk_instance, m_debug_messenger, nullptr);
        }
        vkDestroySurfaceKHR(m_vk_instance, m_surface, nullptr);
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