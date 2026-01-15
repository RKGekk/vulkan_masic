#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "vulkan_instance_layers_and_extensions.h"

class VulkanDeviceExtensions {
public:
    bool init(VkPhysicalDevice physical_device, std::unordered_set<std::string> requested_extensions);

    VkPhysicalDevice getPhysicalDevice();

    bool isExtensionSupported(const LayerExtension::ExtensionName& ext_name) const;

    const std::unordered_set<std::string>& getRequestedExtensions() const;
    const std::unordered_set<std::string>& getExtensionsDiff() const;

    std::vector<const char*> get_ppNames() const;

    template<typename ContainerType>
    static ContainerType getDeviceExtensionsFn(VkPhysicalDevice physical_device, const char* layer_name, std::function<typename ContainerType::value_type(const VkExtensionProperties&)> fn) {
        uint32_t ext_count = 0u;
        VkResult result = vkEnumerateDeviceExtensionProperties(physical_device, layer_name, &ext_count, nullptr);
        if(result != VK_SUCCESS) {
            throw std::runtime_error("failed to read instance properties!");
        }
        std::vector<VkExtensionProperties> ext_props(ext_count);
        result = vkEnumerateDeviceExtensionProperties(physical_device, layer_name, &ext_count, ext_props.data());
        if(result != VK_SUCCESS) {
            throw std::runtime_error("failed to read instance properties!");
        }
        ContainerType available_ext;
        available_ext.reserve(ext_count);
        for(const VkExtensionProperties& prop : ext_props) {
            insert_in_container(available_ext, fn(prop));
        }
        
        return available_ext;
    }

    std::vector<LayerExtension> getDeviceExtensionsAPI(VkPhysicalDevice physical_device, const char* layer_name, std::function<void(const LayerExtension&)> fn = [](const LayerExtension&){});

private:
    VkPhysicalDevice m_physical_device;

    std::unordered_set<std::string> m_requested_extensions;
    std::unordered_set<std::string> m_diff_instance_extensions;

    std::unordered_map<LayerProperty::LayerName, LayerPropertyExt> m_layers;
    std::unordered_map<LayerExtension::ExtensionName, LayerPropertyExt&> m_extensions;
};