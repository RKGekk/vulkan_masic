#pragma once

#include <cstdint>
#include <iostream>
#include <string>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "vulkan_instance_layers_and_extensions.h"

class VulkanInstance {
public:
    bool init(std::string app_name);
    void destroy();

    VkInstance getInstance();
    const VulkanInstanceLayersAndExtensions& getLayersAndExtensions();

private:
    static uint32_t getVkApiVersion();
    static void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& messenger_info);
    static VkDebugUtilsMessengerEXT setupDebugMessanger(VkInstance instance);
    static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* p_create_info, const VkAllocationCallbacks* p_allocator, VkDebugUtilsMessengerEXT* p_debug_messenger);
    static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debug_messenger, const VkAllocationCallbacks* p_allocator);

    template<typename Container>
    static Container getRequiredValidationLayers() {
        using namespace std::literals;
		Container layers;
#ifndef NDEBUG
		insert_in_container(layers, "VK_LAYER_KHRONOS_validation"s);
#endif
		return layers;
	}

    template<typename Container>
    static Container getRequiredInstanceExtensions() {
		uint32_t glfw_extension_count = 0;
		const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);
        Container extensions(glfw_extensions, glfw_extensions + glfw_extension_count);
#ifdef __APPLE__
        insert_in_container(extensions, VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
#endif
#ifndef NDEBUG
        insert_in_container(extensions, VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        insert_in_container(extensions, VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
#endif
		return extensions;
	}

    std::string m_app_name;
    std::string m_engine_name;
    VkInstance m_instance = VK_NULL_HANDLE;
	VulkanInstanceLayersAndExtensions m_layers_and_extensions;
    VkDebugUtilsMessengerEXT m_debug_messenger = VK_NULL_HANDLE;
};