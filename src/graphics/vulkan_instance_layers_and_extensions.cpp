#include "vulkan_instance_layers_and_extensions.h"

bool VulkanInstanceLayersAndExtensions::init(std::unordered_set<std::string> requested_layers, std::unordered_set<std::string> requested_extensions) {
    using namespace std::literals;
    m_requested_layers = std::move(requested_layers);
    m_requested_extensions = std::move(requested_extensions);
    std::vector<LayerProperty> all_layers = getInstanceLayersAPI();
    std::unordered_set<std::string> available_layer_names;
    std::unordered_set<std::string> available_extension_names;
    for(const LayerProperty& vk_layer : all_layers) {
        available_layer_names.insert(vk_layer.layer_name);
        LayerPropertyExt layer_property_and_ext{};
        layer_property_and_ext.property = vk_layer;
        layer_property_and_ext.extensions = getInstanceExtensionsAPI(
            layer_property_and_ext.property.layer_name.data(),
            [&available_extension_names](const LayerExtension& ext) {
                available_extension_names.insert(ext.extension_name);
            }
        );
        auto layer_ext_it = m_layers.emplace(vk_layer.layer_name, std::move(layer_property_and_ext));
        
        for (const LayerExtension& ext : layer_ext_it.first->second.extensions) {
            m_extensions.emplace(ext.extension_name, std::ref(layer_ext_it.first->second));
        }
    }
    std::vector<LayerExtension> empty_layer_ext = getInstanceExtensionsAPI(
        nullptr,
        [&available_extension_names](const LayerExtension& ext) {
            available_extension_names.insert(ext.extension_name);
        }
    );
    std::string empty_layer_name = "VK_LAYER_NULL_HANDLE"s;
    LayerProperty empty_layer_property{};
    empty_layer_property.layer_name = empty_layer_name;
    empty_layer_property.spec_version = 1u;
    empty_layer_property.implementation_version = 1u;
    empty_layer_property.description = "Layer for extensions wo layer. Only extensions provided by the Vulkan implementation or by implicitly enabled layers"s;
    auto empty_layer_ext_it = m_layers.emplace(empty_layer_name, LayerPropertyExt {std::move(empty_layer_property), std::move(empty_layer_ext)});
    for (const LayerExtension& ext : empty_layer_ext_it.first->second.extensions) {
        m_extensions.emplace(ext.extension_name, std::ref(empty_layer_ext_it.first->second));
    }

    bool is_all_layers_supported = checkNamesSupported(available_layer_names, m_requested_layers);
    if(!is_all_layers_supported) {
        m_diff_instance_layers = getNamesUnsupported(available_layer_names, m_requested_layers);
    }

    bool is_all_ext_supported = checkNamesSupported(available_extension_names, m_requested_extensions);
    if(!is_all_ext_supported) {
        m_diff_instance_extensions = getNamesUnsupported(available_extension_names, m_requested_extensions);
    }

    return is_all_layers_supported && is_all_ext_supported;
}

const std::unordered_map<LayerProperty::LayerName, LayerPropertyExt>& VulkanInstanceLayersAndExtensions::getInstanceLayers() const {
    return m_layers;
}

const std::vector<LayerExtension>& VulkanInstanceLayersAndExtensions::getInstanceLayerExtensions(const LayerProperty::LayerName& layer_name) const {
    return m_layers.at(layer_name).extensions;
}

bool VulkanInstanceLayersAndExtensions::isLayerSupported(const LayerProperty::LayerName& layer_name) const {
    return m_layers.count(layer_name);
}

bool VulkanInstanceLayersAndExtensions::isExtensionSupported(const LayerExtension::ExtensionName& ext_name) const {
    return m_extensions.count(ext_name);
}

const std::unordered_set<std::string>& VulkanInstanceLayersAndExtensions::getRequestedLayers() const {
    return m_requested_layers;
}

const std::unordered_set<std::string>& VulkanInstanceLayersAndExtensions::getRequestedExtensions() const {
    return m_requested_extensions;
}

const std::unordered_set<std::string>& VulkanInstanceLayersAndExtensions::getLayersDiff() const {
    return m_diff_instance_layers;
}

const std::unordered_set<std::string>& VulkanInstanceLayersAndExtensions::getExtensionsDiff() const {
    return m_diff_instance_extensions;
}

std::vector<const char*> VulkanInstanceLayersAndExtensions::get_ppLayerNames() const {
    std::vector<const char*> req_layers_ppNames;
    req_layers_ppNames.reserve(m_requested_layers.size());
    std::transform(m_requested_layers.cbegin(), m_requested_layers.cend(), std::inserter(req_layers_ppNames, req_layers_ppNames.end()), [&](const std::string& str) { return str.c_str(); });
    return req_layers_ppNames;
}

std::vector<const char*> VulkanInstanceLayersAndExtensions::get_ppExtNames() const {
    std::vector<const char*> req_ext_ppNames;
    req_ext_ppNames.reserve(m_requested_extensions.size());
    std::transform(m_requested_extensions.cbegin(), m_requested_extensions.cend(), std::inserter(req_ext_ppNames, req_ext_ppNames.end()), [&](const std::string& str) { return str.c_str(); });
    return req_ext_ppNames;
}

std::vector<LayerProperty> VulkanInstanceLayersAndExtensions::getInstanceLayersAPI() {
    std::vector<LayerProperty> all_layers = getInstanceLayersFn<std::vector<LayerProperty>>(
        [](const VkLayerProperties& layer_prop){
            LayerProperty layer_property{};
            layer_property.layer_name = std::string(layer_prop.layerName);
            layer_property.spec_version = layer_prop.specVersion;
            layer_property.implementation_version = layer_prop.implementationVersion;
            layer_property.description = std::string(layer_prop.description);
            return layer_property;
        }
    );
    return all_layers;
}

std::vector<LayerExtension> VulkanInstanceLayersAndExtensions::getInstanceExtensionsAPI(const char* layer_name, std::function<void(const LayerExtension&)> fn) {
    std::vector<LayerExtension> extensions = getInstanceExtensionsFn<std::vector<LayerExtension>>(
        layer_name,
        [&fn](const VkExtensionProperties& ext_prop) {
            LayerExtension ext;
            ext.extension_name = std::string(ext_prop.extensionName);
            ext.spec_version = ext_prop.specVersion;
            fn(ext);
            return ext;
        }
    );
    return extensions;
};