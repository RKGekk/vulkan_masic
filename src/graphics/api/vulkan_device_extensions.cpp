#include "vulkan_device_extensions.h"

bool VulkanDeviceExtensions::init(VkPhysicalDevice physical_device, std::unordered_set<std::string> requested_extensions) {
    using namespace std::literals;

    m_physical_device = physical_device;
    m_requested_extensions = std::move(requested_extensions);

    std::vector<LayerProperty> all_layers = VulkanInstanceLayersAndExtensions::getInstanceLayersAPI();
    std::unordered_set<std::string> available_layer_names;
    std::unordered_set<std::string> available_extension_names;
    for(const LayerProperty& vk_layer : all_layers) {
        available_layer_names.insert(vk_layer.layer_name);
        LayerPropertyExt layer_property_and_ext{};
        layer_property_and_ext.property = vk_layer;
        layer_property_and_ext.extensions = getDeviceExtensionsAPI(
            m_physical_device,
            layer_property_and_ext.property.layer_name.data(),
            [&available_extension_names](const LayerExtension& ext) {
                available_extension_names.insert(ext.extension_name);
            }
        );
        if (!layer_property_and_ext.extensions.size()) {
            continue;
        }
        auto layer_ext_it = m_layers.emplace(vk_layer.layer_name, std::move(layer_property_and_ext));
        
        for (const LayerExtension& ext : layer_ext_it.first->second.extensions) {
            m_extensions.emplace(ext.extension_name, std::ref(layer_ext_it.first->second));
        }
    }
    std::vector<LayerExtension> empty_layer_ext = getDeviceExtensionsAPI(
        m_physical_device,
        nullptr,
        [&available_extension_names](const LayerExtension& ext) {
            available_extension_names.insert(ext.extension_name);
        }
    );
    if(empty_layer_ext.size()) {
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
    }

    bool is_all_ext_supported = checkNamesSupported(available_extension_names, m_requested_extensions);
    if(!is_all_ext_supported) {
        m_diff_instance_extensions = getNamesUnsupported(available_extension_names, m_requested_extensions);
    }

    return is_all_ext_supported;
}

VkPhysicalDevice VulkanDeviceExtensions::getPhysicalDevice() {
    return m_physical_device;
}

const std::unordered_set<std::string>& VulkanDeviceExtensions::getRequestedExtensions() const {
    return m_requested_extensions;
}

const std::unordered_set<std::string>& VulkanDeviceExtensions::getExtensionsDiff() const {
    return m_diff_instance_extensions;
}

std::vector<const char*> VulkanDeviceExtensions::get_ppNames() const {
    std::vector<const char*> req_device_extensions_ppNames;
    req_device_extensions_ppNames.reserve(m_requested_extensions.size());
    std::transform(m_requested_extensions.cbegin(), m_requested_extensions.cend(), std::inserter(req_device_extensions_ppNames, req_device_extensions_ppNames.begin()), [&](const std::string& str) { return str.c_str(); });
    return req_device_extensions_ppNames;
}

std::vector<LayerExtension> VulkanDeviceExtensions::getDeviceExtensionsAPI(VkPhysicalDevice physical_device, const char* layer_name, std::function<void(const LayerExtension&)> fn) {
    std::vector<LayerExtension> extensions = getDeviceExtensionsFn<std::vector<LayerExtension>>(
        physical_device,
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