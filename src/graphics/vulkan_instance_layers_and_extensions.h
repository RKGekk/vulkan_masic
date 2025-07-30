#pragma once

#include <functional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <vulkan/vulkan.h>

#include "../tools/string_tools.h"

struct LayerProperty {
    std::string layer_name;
    uint32_t spec_version;
    uint32_t implementation_version;
    std::string description;
};

struct LayerExtension {
    std::string extension_name;
    uint32_t spec_version;
};

struct LayerPropertyExt{
    LayerProperty property;
    std::vector<LayerExtension> extensions;
};

class VulkanInstanceLayersAndExtensions {
public:
    bool init(std::unordered_set<std::string> requested_layers, std::unordered_set<std::string> requested_extensions);

    using LayerName = std::string;
    using ExtensionName = std::string;

    const std::unordered_map<LayerName, LayerPropertyExt>& getInstanceLayers() const;
    const std::vector<LayerExtension>& getInstanceLayerExtensions(const LayerName& layer_name) const;
    bool isLayerSupported(const LayerName& layer_name) const;
    bool isLayersSupported(const LayerName& layer_name) const;
    bool isExtensionSupported(const ExtensionName& ext_name) const;

    template<typename ContainerType>
    static ContainerType getInstanceLayers(std::function<typename ContainerType::value_type(const VkLayerProperties&)> fn) {
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

        ContainerType validation_layers;
        validation_layers.reserve(layers_count);
        for(const VkLayerProperties& prop : layer_props) {
            insert_in_container(validation_layers, fn(prop));
        }
        
        return validation_layers;
    }

    template<typename ContainerType>
    static ContainerType getInstanceExtensions(const char* layer_name, std::function<typename ContainerType::value_type(const VkExtensionProperties&)> fn) {
        uint32_t ext_count = 0u;
        VkResult result = vkEnumerateInstanceExtensionProperties(layer_name, &ext_count, nullptr);
        if(result != VK_SUCCESS) {
            throw std::runtime_error("failed to read instance properties!");
        }
        std::vector<VkExtensionProperties> ext_props(ext_count);
        result = vkEnumerateInstanceExtensionProperties(layer_name, &ext_count, ext_props.data());
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

    const std::unordered_set<std::string>& getRequestedLayers() const;
    const std::unordered_set<std::string>& getRequestedExtensions() const;

    const std::unordered_set<std::string>& getLayersDiff() const;
    const std::unordered_set<std::string>& getExtensionsDiff() const;

    std::vector<const char*> get_ppLayerNames() const;
    std::vector<const char*> get_ppExtNames() const;

private:
    std::unordered_set<std::string> m_requested_layers;
    std::unordered_set<std::string> m_diff_instance_layers;

    std::unordered_set<std::string> m_requested_extensions;
    std::unordered_set<std::string> m_diff_instance_extensions;

    std::unordered_map<LayerName, LayerPropertyExt> m_layers;
    std::unordered_map<ExtensionName, LayerPropertyExt&> m_extensions;
};