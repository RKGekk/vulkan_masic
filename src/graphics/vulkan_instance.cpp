#include "vulkan_instance.h"

#include <stdexcept>
#include <unordered_set>

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
    std::cout << "validation layer: " << pCallbackData->pMessage << std::endl;
    
    return VK_FALSE;
}

bool VulkanInstance::init(std::string app_name) {
    m_app_name = std::move(app_name);
    bool is_all_ext_supported = m_layers_and_extensions.init(getRequiredValidationLayers<std::unordered_set<std::string>>(), getRequiredInstanceExtensions<std::unordered_set<std::string>>());

#ifndef NDEBUG
    for (const auto&[layer_name, layer_prop] : m_layers_and_extensions.getInstanceLayers()) {
        std::cout << layer_name << std::endl;
        for (const LayerExtension& layer_ext : layer_prop.extensions) {
            std::cout << " - " << layer_ext.extension_name << std::endl;
        }
    }
#endif
    if(!is_all_ext_supported) {
        for(const std::string& name : m_layers_and_extensions.getLayersDiff()) {
            throw std::runtime_error("instance not support layer - " + name);
        }
        for(const std::string& name : m_layers_and_extensions.getExtensionsDiff()) {
            throw std::runtime_error("instance not support extension - " + name);
        }
    }

    VkApplicationInfo app_info{};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = m_app_name.data();
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName = "No Engine";
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.apiVersion = getVkApiVersion();

    VkInstanceCreateInfo instance_info{};
    instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_info.pApplicationInfo = &app_info;

    const std::vector<const char*> req_instance_layer_ppNames = m_layers_and_extensions.get_ppLayerNames();
    instance_info.ppEnabledLayerNames = req_instance_layer_ppNames.data();
    instance_info.enabledLayerCount = static_cast<uint32_t>(req_instance_layer_ppNames.size());
        
    std::vector<const char*> req_instance_extensions_ppNames = m_layers_and_extensions.get_ppExtNames();
    instance_info.ppEnabledExtensionNames = req_instance_extensions_ppNames.data();
    instance_info.enabledExtensionCount = static_cast<uint32_t>(req_instance_extensions_ppNames.size());

#ifndef NDEBUG
    VkDebugUtilsMessengerCreateInfoEXT messenger_info{};
    populateDebugMessengerCreateInfo(messenger_info);
    instance_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &messenger_info;
#endif
        
#ifdef __APPLE__
    instance_info.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#else
    instance_info.flags = 0u;
#endif
        
    VkResult result = vkCreateInstance(&instance_info, nullptr, &m_instance);
    if(result != VK_SUCCESS) {
        throw std::runtime_error("failed to create instance!");
    }

#ifndef NDEBUG
    m_debug_messenger = setupDebugMessanger(m_instance);
#endif

    return is_all_ext_supported;
}

void VulkanInstance::destroy() {
#ifndef NDEBUG
    DestroyDebugUtilsMessengerEXT(m_instance, m_debug_messenger, nullptr);
#endif
    vkDestroyInstance(m_instance, nullptr);
}

VkInstance VulkanInstance::getInstance() const {
    return m_instance;
}

const VulkanInstanceLayersAndExtensions& VulkanInstance::getLayersAndExtensions() const {
    return m_layers_and_extensions;
}

uint32_t VulkanInstance::getVkApiVersion() {
    uint32_t api_version = VK_API_VERSION_1_0;
    VkResult result = vkEnumerateInstanceVersion(&api_version);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to get Vulkan API version!");
    }
    return api_version;
}

void VulkanInstance::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& messenger_info) {
        messenger_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        messenger_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        messenger_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        messenger_info.pfnUserCallback = debugCallback;
        messenger_info.pUserData = nullptr;
    }

VkDebugUtilsMessengerEXT VulkanInstance::setupDebugMessanger(VkInstance instance) {
        VkDebugUtilsMessengerCreateInfoEXT messenger_info{};
        populateDebugMessengerCreateInfo(messenger_info);
        
        VkDebugUtilsMessengerEXT debug_messanger = VK_NULL_HANDLE;
        VkResult result = CreateDebugUtilsMessengerEXT(instance, &messenger_info, nullptr, &debug_messanger);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to set up debug messenger!");
        }
        
        return debug_messanger;
}

VkResult VulkanInstance::CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* p_create_info, const VkAllocationCallbacks* p_allocator, VkDebugUtilsMessengerEXT* p_debug_messenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, p_create_info, p_allocator, p_debug_messenger);
    }
    else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void VulkanInstance::DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debug_messenger, const VkAllocationCallbacks* p_allocator) {
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) {
		func(instance, debug_messenger, p_allocator);
	}
}