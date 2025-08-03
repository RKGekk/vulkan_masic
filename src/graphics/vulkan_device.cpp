#include "vulkan_device.h"

std::vector<VkQueueFamilyProperties> QueueFamilyIndices::getQueueFamilies(VkPhysicalDevice device) {
    uint32_t queue_family_count = 0u;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);
    
    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families.data());

    return queue_families;
}

void QueueFamilyIndices::init(VkPhysicalDevice device, VkSurfaceKHR surface) {
    queue_families = getQueueFamilies(device);
    
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

bool QueueFamilyIndices::isComplete() const {
    return graphics_family.has_value() && present_family.has_value() && compute_family.has_value() && transfer_family.has_value();
}

VkSharingMode QueueFamilyIndices::getBufferSharingMode() const {
    if (graphics_family.has_value() && transfer_family.has_value() && graphics_family.value() != transfer_family.value()) {
        return VK_SHARING_MODE_CONCURRENT;
    }
    return VK_SHARING_MODE_EXCLUSIVE;
}

std::unordered_set<uint32_t> QueueFamilyIndices::getFamilies() const {
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

std::vector<uint32_t> QueueFamilyIndices::getIndices() const {
    const auto& families = getFamilies();
    std::vector<uint32_t> result(families.cbegin(), families.cend());
    return result;
}

bool VulkanDevice::init(const VulkanInstance& instance, VkSurfaceKHR surface) {
    m_device_abilities = pickPhysicalDevice(instance.getInstance(), surface);
    bool all_device_ext_supported = m_extensions.init(m_device_abilities.physical_device, getRequiredDeviceExtensions<std::unordered_set<std::string>>());

#ifndef NDEBUG
        printInfo("Supported device extensions", m_extensions.getRequestedExtensions());
#endif

    m_msaa_samples = getMaxUsableSampleCount(m_device_abilities.props);
    m_queue_family_indices.init(m_device_abilities.physical_device, surface);
    m_device = createLogicalDevice(m_device_abilities.physical_device, m_queue_family_indices, m_extensions, instance.getLayersAndExtensions());

    vkGetDeviceQueue(m_device, m_queue_family_indices.graphics_family.value(), 0u, &m_graphics_queue);
    vkGetDeviceQueue(m_device, m_queue_family_indices.present_family.value(), 0u, &m_present_queue);
    vkGetDeviceQueue(m_device, m_queue_family_indices.compute_family.value(), 0u, &m_compute_queue);
    vkGetDeviceQueue(m_device, m_queue_family_indices.transfer_family.value(), 0u, &m_transfer_queue);

    return all_device_ext_supported;
}

void VulkanDevice::destroy() {
    vkDestroyDevice(m_device, nullptr);
}

const VulkanDeviceExtensions& VulkanDevice::getExtensions() const {
    return m_extensions;
}

VkDevice VulkanDevice::getDevice() const {
    return m_device;
}

const DeviceAbilities& VulkanDevice::getDeviceAbilities() const {
    return m_device_abilities;
}

const QueueFamilyIndices& VulkanDevice::getQueueFamilyIndices() const {
    return m_queue_family_indices;
}
    
VkQueue VulkanDevice::getGraphicsQueue() const {
    return m_graphics_queue;
}

VkQueue VulkanDevice::getComputeQueue() const {
    return m_compute_queue;
}

VkQueue VulkanDevice::getTransferQueue() const {
    return m_transfer_queue;
}
    
VkQueue VulkanDevice::getPresentQueue() const {
    return m_present_queue;
}

VkSampleCountFlagBits VulkanDevice::getMsaaSamples() const {
    return m_msaa_samples;
}

DeviceAbilities VulkanDevice::pickPhysicalDevice(VkInstance vk_instance, VkSurfaceKHR surface) {
    std::vector<VkPhysicalDevice> devices = getPhysicalDevices(vk_instance);
    
    std::multimap<int, DeviceAbilities> candidates;
    for(VkPhysicalDevice device : devices){
        DeviceAbilities device_ailities = rateDeviceSuitability(device, surface);
        candidates.insert(std::make_pair(device_ailities.score, device_ailities));
    }
    
    DeviceAbilities physical_device{};
    if (candidates.size() > 0u) {
        physical_device = candidates.rbegin()->second;
    }
    else {
        throw std::runtime_error("failed to find a suitable GPU!");
    }
    
    return physical_device;
}

std::vector<VkPhysicalDevice> VulkanDevice::getPhysicalDevices(VkInstance vk_instance) {
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

DeviceAbilities VulkanDevice::rateDeviceSuitability(VkPhysicalDevice device, VkSurfaceKHR surface) {
    DeviceAbilities device_abilities{};

    device_abilities.physical_device = device;
    vkGetPhysicalDeviceProperties(device, &device_abilities.props);
    vkGetPhysicalDeviceFeatures(device, &device_abilities.features);
    vkGetPhysicalDeviceMemoryProperties(device, &device_abilities.memory_properties);
    
    if(device_abilities.props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
        device_abilities.score += 1000;
    }
    device_abilities.score += device_abilities.props.limits.maxImageDimension2D;

    uint64_t max_memory = getDeviceMaxMemoryLimit(device_abilities.memory_properties);
    device_abilities.score += (int)std::log2(max_memory);
    
    if(device_abilities.features.geometryShader) {
        device_abilities.score += 1000;
    }
    if(!isDeviceSuitable(device, surface)){
        device_abilities.score = 0;
    }
    
    return device_abilities;
}

uint64_t VulkanDevice::getDeviceMaxMemoryLimit(VkPhysicalDeviceMemoryProperties phys_device_mem_prop) {
    uint64_t max_memory_limit = 0;
    for (uint32_t i = 0u; i < phys_device_mem_prop.memoryHeapCount; ++i) {
        VkMemoryHeap mem_heap_prop = phys_device_mem_prop.memoryHeaps[i];
        max_memory_limit += mem_heap_prop.size;
    }
    return max_memory_limit;
}

bool VulkanDevice::isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface) {
    VkPhysicalDeviceFeatures supported_features{};
    vkGetPhysicalDeviceFeatures(device, &supported_features);

    QueueFamilyIndices queue_family_indices;
    queue_family_indices.init(device, surface);
    
    VulkanDeviceExtensions dev_ext;
    bool all_device_ext_supported = dev_ext.init(device, getRequiredDeviceExtensions<std::unordered_set<std::string>>());

    bool all_queue_families_supported = queue_family_indices.isComplete();
    
    bool swap_chain_adequate = false;
    if(all_device_ext_supported) {
        SwapchainSupportDetails swap_chain_details = querySwapChainSupport(device, surface);
        swap_chain_adequate = !swap_chain_details.formats.empty() && !swap_chain_details.present_modes.empty();
    }
    
    bool result = all_queue_families_supported && all_device_ext_supported && swap_chain_adequate && supported_features.samplerAnisotropy;
    return result;
}

VkSampleCountFlagBits VulkanDevice::getMaxUsableSampleCount(VkPhysicalDeviceProperties physical_device_properties) {
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

VkDevice VulkanDevice::createLogicalDevice(VkPhysicalDevice physical_device, const QueueFamilyIndices& queue_family_indices, const VulkanDeviceExtensions& device_extensions, const VulkanInstanceLayersAndExtensions& layers) {
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

    std::vector<const char*> validation_layers_ppNames = layers.get_ppLayerNames();
#ifndef NDEBUG
    device_create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers_ppNames.size());
    device_create_info.ppEnabledLayerNames = validation_layers_ppNames.data();
#else
    device_create_info.enabledLayerCount = 0u;
#endif

    VkDevice device;
    VkResult result = vkCreateDevice(physical_device, &device_create_info, nullptr, &device);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to create logical device!");
    }
    
    return device;
}