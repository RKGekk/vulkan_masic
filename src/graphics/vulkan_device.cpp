#include "vulkan_device.h"
#include "vulkan_swapchain.h"
#include "vulkan_queue_family.h"
#include "vulkan_command_manager.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

ImageBufferAndView ImageBuffer::getImageAndView() const {
    return {memory, image, image_info, image_size};
}

ImageBuffer ImageBufferAndView::getImage() const {
    return {memory, image, image_info, image_size};
}

bool VulkanDevice::init(const VulkanInstance& instance, VkSurfaceKHR surface) {
    m_device_abilities = pickPhysicalDevice(instance.getInstance(), surface);
    bool all_device_ext_supported = m_extensions.init(m_device_abilities.physical_device, getRequiredDeviceExtensions<std::unordered_set<std::string>>());

#ifndef NDEBUG
        printInfo("Supported device extensions", m_extensions.getRequestedExtensions());
#endif

    m_msaa_samples = getMaxUsableSampleCount(m_device_abilities.props);

    QueueFamilyIndices queue_family_indices;
    queue_family_indices.init(m_device_abilities.physical_device, surface);
    m_device = createLogicalDevice(m_device_abilities.physical_device, queue_family_indices.getFamilies(), m_extensions, instance.getLayersAndExtensions());
    m_command_manager.init(m_device_abilities.physical_device, m_device, surface);

    return all_device_ext_supported;
}

void VulkanDevice::destroy() {
    m_command_manager.destroy();
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

VkSampleCountFlagBits VulkanDevice::getMsaaSamples() const {
    return m_msaa_samples;
}

const VulkanCommandManager& VulkanDevice::getCommandManager() const {
    return m_command_manager;
}

VulkanBuffer VulkanDevice::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) const {
    VulkanBuffer vulkan_buffer;
    vulkan_buffer.properties = properties;

    std::vector<uint32_t> family_indices = m_command_manager.getQueueFamilyIndices().getIndices();

    VkBufferCreateInfo buffer_info{};
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size = size;
    buffer_info.usage = usage;
    buffer_info.sharingMode = m_command_manager.getQueueFamilyIndices().getBufferSharingMode();
    buffer_info.queueFamilyIndexCount = static_cast<uint32_t>(family_indices.size());
    buffer_info.pQueueFamilyIndices = family_indices.data();
    
    VkResult result = vkCreateBuffer(m_device, &buffer_info, nullptr, &vulkan_buffer.buf);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to create buffer!");
    }
    
    VkMemoryRequirements mem_req;
    vkGetBufferMemoryRequirements(m_device, vulkan_buffer.buf, &mem_req);
    uint32_t mem_type_idx = findMemoryType(mem_req.memoryTypeBits, properties);

    VkMemoryAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = mem_req.size;
    alloc_info.memoryTypeIndex = mem_type_idx;
    
    result = vkAllocateMemory(m_device, &alloc_info, nullptr, &vulkan_buffer.mem);
    if(result != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate buffer memory!");
    }
    
    VkDeviceSize offset = 0u;

    vkBindBufferMemory(m_device, vulkan_buffer.buf, vulkan_buffer.mem, offset);

    vulkan_buffer.bufferInfo.buffer = vulkan_buffer.buf;
    vulkan_buffer.bufferInfo.offset = offset;
    vulkan_buffer.bufferInfo.range = mem_req.size;

    return vulkan_buffer;
}

VkImageView VulkanDevice::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspect_flags, uint32_t mip_levels) const {
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

std::vector<VkImageView> VulkanDevice::createImageViews(const std::vector<VkImage>& images, VkFormat format, VkImageAspectFlags aspect_flags, uint32_t mip_levels) const {
    uint32_t sz = static_cast<uint32_t>(images.size());
    std::vector<VkImageView> image_views(sz);
    for(uint32_t i = 0u; i < sz; ++i) {
        image_views[i] = createImageView(images[i], format, aspect_flags, mip_levels);
    }
    
    return image_views;
}

ImageBuffer VulkanDevice::createImage(const VkImageCreateInfo& image_info, VkMemoryPropertyFlags properties) const {
    ImageBuffer image_buffer{};
    image_buffer.image_info = image_info;
    VkResult result = vkCreateImage(m_device, &image_info, nullptr, &image_buffer.image);
    if(result != VK_SUCCESS) {
        throw std::runtime_error("failed to create image!");
    }
    
    VkMemoryRequirements mem_req{};
    vkGetImageMemoryRequirements(m_device, image_buffer.image, &mem_req);
    image_buffer.image_size = mem_req.size;
    
    uint32_t mem_type_idx = findMemoryType(mem_req.memoryTypeBits, properties);
    VkMemoryAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = mem_req.size;
    alloc_info.memoryTypeIndex = mem_type_idx;
    
    result = vkAllocateMemory(m_device, &alloc_info, nullptr, &image_buffer.memory);
    if(result != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate image memory!");
    }

    VkDeviceSize offset = 0u;
    vkBindImageMemory(m_device, image_buffer.image, image_buffer.memory, offset);

    return image_buffer;
}

ImageBufferAndView VulkanDevice::createImage(const VkImageCreateInfo& image_info, VkMemoryPropertyFlags properties, VkImageAspectFlags aspect_flags, uint32_t mip_levels) const {
    ImageBuffer image_buffer = createImage(image_info, properties);
    ImageBufferAndView image_and_view = image_buffer.getImageAndView();
    image_and_view.image_view = createImageView(image_buffer.image, image_info.format, aspect_flags, 1u);
    return image_and_view;
}

ImageBuffer VulkanDevice::createImage(const std::string& path_to_file) const {
    int tex_width;
    int tex_height;
    int tex_channels;
    stbi_uc* pixels = stbi_load(path_to_file.c_str(), &tex_width, &tex_height, &tex_channels, STBI_rgb_alpha);
    if (!pixels) {
        throw std::runtime_error("failed to load texture image!");
    }
    VkDeviceSize image_size = tex_width * tex_height * 4;
    
    VulkanBuffer staging_buffer = createBuffer(image_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    
    void* data;
    vkMapMemory(m_device, staging_buffer.mem, 0, image_size, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(image_size));
    vkUnmapMemory(m_device, staging_buffer.mem);
    
    stbi_image_free(pixels);

    uint32_t mip_levels = static_cast<uint32_t>(std::floor(std::log2(std::max(tex_width, tex_height)))) + 1u;

    VkFormat image_format = findSupportedFormat(
        {
            VK_FORMAT_R8G8B8A8_SRGB
        },
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT
    );
    
    std::vector<uint32_t> families = m_command_manager.getQueueFamilyIndices().getIndices();
    VkImageCreateInfo image_info{};
    image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.extent.width = static_cast<uint32_t>(tex_width);
    image_info.extent.height = static_cast<uint32_t>(tex_height);
    image_info.extent.depth = 1u;
    image_info.mipLevels = mip_levels;
    image_info.arrayLayers = 1u;
    //image_info.format = VK_FORMAT_R8G8B8A8_SRGB;
    image_info.format = image_format;
    image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    image_info.sharingMode = m_command_manager.getBufferSharingMode();
    image_info.queueFamilyIndexCount = static_cast<uint32_t>(families.size());
    image_info.pQueueFamilyIndices = families.data();
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_info.flags = 0u;
    
    ImageBuffer image = createImage(image_info, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    
    m_command_manager.transitionImageLayout(image.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mip_levels);
    m_command_manager.copyBufferToImage(staging_buffer.buf, image.image, static_cast<uint32_t>(tex_width), static_cast<uint32_t>(tex_height));
    m_command_manager.generateMipmaps(image.image, VK_FORMAT_R8G8B8A8_SRGB, tex_width, tex_height, mip_levels);
    //m_command_manager.transitionImageLayout(image.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, mip_levels);
    
    vkDestroyBuffer(m_device, staging_buffer.buf, nullptr);
    vkFreeMemory(m_device, staging_buffer.mem, nullptr);
    
    return image;
}

ImageBufferAndView VulkanDevice::createImageAndView(const std::string& path_to_file) const {
    ImageBuffer image_buffer = createImage(path_to_file);
    ImageBufferAndView image_and_view = image_buffer.getImageAndView();
    image_and_view.image_view = createImageView(image_buffer.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, image_buffer.image_info.mipLevels);
    return image_and_view;
}

uint32_t VulkanDevice::findMemoryType(uint32_t type_filter, VkMemoryPropertyFlags properties) const {
    VkPhysicalDeviceMemoryProperties mem_prop{};
    vkGetPhysicalDeviceMemoryProperties(getDeviceAbilities().physical_device, &mem_prop);
    for(uint32_t i = 0u; i < mem_prop.memoryTypeCount; ++i) {
        bool is_type_suit = type_filter & (1 << i);
        bool is_type_adequate = mem_prop.memoryTypes[i].propertyFlags & properties;
        if(is_type_suit && is_type_adequate) {
            return i;
        }
    }
            
    throw std::runtime_error("failed to find suitable memory type!");
}

VkFormat VulkanDevice::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) const {
    for (VkFormat format : candidates) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(m_device_abilities.physical_device, format, &props);
        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
            return format;
        }
        else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }
    throw std::runtime_error("failed to find supported format!");
}

VkFormat VulkanDevice::findDepthFormat() {
    return findSupportedFormat(
        {
            VK_FORMAT_D32_SFLOAT_S8_UINT,
            VK_FORMAT_D24_UNORM_S8_UINT,
            VK_FORMAT_D32_SFLOAT,
        },
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
}

bool VulkanDevice::hasStencilComponent(VkFormat format) {
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
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

    device_abilities.host_visible_single_heap_memory = isHostVisibleSingleHeapMemory(device);
    
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
        SwapchainSupportDetails swap_chain_details = VulkanSwapChain::querySwapChainSupport(device, surface);
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

VkDevice VulkanDevice::createLogicalDevice(VkPhysicalDevice physical_device, const std::unordered_set<uint32_t>& family_indices, const VulkanDeviceExtensions& device_extensions, const VulkanInstanceLayersAndExtensions& layers) {
    std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
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

bool VulkanDevice::isHostVisibleSingleHeapMemory(VkPhysicalDevice physical_device) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physical_device, &memProperties);
    if (memProperties.memoryHeapCount != 1) {
        return false;
    }

    const uint32_t flag = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    for (uint32_t i = 0u; i < memProperties.memoryTypeCount; ++i) {
        if ((memProperties.memoryTypes[i].propertyFlags & flag) == flag) {
            return true;
        }
    }

    return false;
}