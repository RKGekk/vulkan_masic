#include "vulkan_device.h"
#include "vulkan_swapchain.h"
#include "vulkan_queue_family.h"
#include "vulkan_command_manager.h"

bool VulkanDevice::init(const VulkanInstance& instance, VkSurfaceKHR surface, std::shared_ptr<ThreadPool> thread_pool) {
    m_thread_pool = std::move(thread_pool);
    m_device_abilities = pickPhysicalDevice(instance.getInstance(), surface);
    bool all_device_ext_supported = m_extensions.init(m_device_abilities.physical_device, getRequiredDeviceExtensions<std::unordered_set<std::string>>());

#ifndef NDEBUG
        printInfo("Supported device extensions", m_extensions.getRequestedExtensions());
#endif

    m_msaa_samples = getMaxUsableSampleCount(m_device_abilities.props);

    QueueFamilyIndices queue_family_indices;
    queue_family_indices.init(m_device_abilities.physical_device, surface);
    m_device = createLogicalDevice(m_device_abilities.physical_device, queue_family_indices.getFamilies(), m_extensions, instance.getLayersAndExtensions());
    m_command_manager.init(m_device_abilities.physical_device, m_device, surface, m_thread_pool);

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

VulkanCommandManager& VulkanDevice::getCommandManager() {
    return m_command_manager;
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

VkAccessFlags VulkanDevice::getDstAccessMask(VkBufferUsageFlags usage) {
    switch (usage) {
        case VK_BUFFER_USAGE_TRANSFER_SRC_BIT: return VK_ACCESS_MEMORY_READ_BIT;
        case VK_BUFFER_USAGE_TRANSFER_DST_BIT: return VK_ACCESS_MEMORY_READ_BIT;
        case VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT: return VK_ACCESS_MEMORY_READ_BIT;
        case VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT: return VK_ACCESS_MEMORY_READ_BIT;
        case VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT: return VK_ACCESS_UNIFORM_READ_BIT;
        case VK_BUFFER_USAGE_STORAGE_BUFFER_BIT: return VK_ACCESS_MEMORY_READ_BIT;
        case VK_BUFFER_USAGE_INDEX_BUFFER_BIT: return VK_ACCESS_MEMORY_READ_BIT;
        case VK_BUFFER_USAGE_VERTEX_BUFFER_BIT: return VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
        case VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT: return VK_ACCESS_MEMORY_READ_BIT;
        case VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT: return VK_ACCESS_MEMORY_READ_BIT;
        case VK_BUFFER_USAGE_VIDEO_DECODE_SRC_BIT_KHR: return VK_ACCESS_MEMORY_READ_BIT;
        case VK_BUFFER_USAGE_VIDEO_DECODE_DST_BIT_KHR: return VK_ACCESS_MEMORY_READ_BIT;
        case VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_BUFFER_BIT_EXT: return VK_ACCESS_MEMORY_READ_BIT;
        case VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_COUNTER_BUFFER_BIT_EXT: return VK_ACCESS_MEMORY_READ_BIT;
        case VK_BUFFER_USAGE_CONDITIONAL_RENDERING_BIT_EXT: return VK_ACCESS_MEMORY_READ_BIT;
        case VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR: return VK_ACCESS_MEMORY_READ_BIT;
        case VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR: return VK_ACCESS_MEMORY_READ_BIT;
        case VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR: return VK_ACCESS_MEMORY_READ_BIT;
        case VK_BUFFER_USAGE_VIDEO_ENCODE_DST_BIT_KHR: return VK_ACCESS_MEMORY_READ_BIT;
        case VK_BUFFER_USAGE_VIDEO_ENCODE_SRC_BIT_KHR: return VK_ACCESS_MEMORY_READ_BIT;
        case VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT: return VK_ACCESS_MEMORY_READ_BIT;
        case VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT: return VK_ACCESS_MEMORY_READ_BIT;
        case VK_BUFFER_USAGE_PUSH_DESCRIPTORS_DESCRIPTOR_BUFFER_BIT_EXT: return VK_ACCESS_MEMORY_READ_BIT;
        case VK_BUFFER_USAGE_MICROMAP_BUILD_INPUT_READ_ONLY_BIT_EXT: return VK_ACCESS_MEMORY_READ_BIT;
        case VK_BUFFER_USAGE_MICROMAP_STORAGE_BIT_EXT: return VK_ACCESS_MEMORY_READ_BIT;
        default: return VK_ACCESS_MEMORY_READ_BIT;
    }
}

size_t VulkanDevice::getBytesCount(VkFormat format) {
    switch (format) {
        case VK_FORMAT_UNDEFINED : return 4;
        case VK_FORMAT_R4G4_UNORM_PACK8 : return 4;
        case VK_FORMAT_R4G4B4A4_UNORM_PACK16 : return 4;
        case VK_FORMAT_B4G4R4A4_UNORM_PACK16 : return 4;
        case VK_FORMAT_R5G6B5_UNORM_PACK16 : return 4;
        case VK_FORMAT_B5G6R5_UNORM_PACK16 : return 4;
        case VK_FORMAT_R5G5B5A1_UNORM_PACK16 : return 4;
        case VK_FORMAT_B5G5R5A1_UNORM_PACK16 : return 4;
        case VK_FORMAT_A1R5G5B5_UNORM_PACK16 : return 4;
        case VK_FORMAT_R8_UNORM : return 4;
        case VK_FORMAT_R8_SNORM : return 4;
        case VK_FORMAT_R8_USCALED : return 4;
        case VK_FORMAT_R8_SSCALED : return 4;
        case VK_FORMAT_R8_UINT : return 4;
        case VK_FORMAT_R8_SINT : return 4;
        case VK_FORMAT_R8_SRGB : return 4;
        case VK_FORMAT_R8G8_UNORM : return 4;
        case VK_FORMAT_R8G8_SNORM : return 4;
        case VK_FORMAT_R8G8_USCALED : return 4;
        case VK_FORMAT_R8G8_SSCALED : return 4;
        case VK_FORMAT_R8G8_UINT : return 4;
        case VK_FORMAT_R8G8_SINT : return 4;
        case VK_FORMAT_R8G8_SRGB : return 4;
        case VK_FORMAT_R8G8B8_UNORM : return 4;
        case VK_FORMAT_R8G8B8_SNORM : return 4;
        case VK_FORMAT_R8G8B8_USCALED : return 4;
        case VK_FORMAT_R8G8B8_SSCALED : return 4;
        case VK_FORMAT_R8G8B8_UINT : return 4;
        case VK_FORMAT_R8G8B8_SINT : return 4;
        case VK_FORMAT_R8G8B8_SRGB : return 4;
        case VK_FORMAT_B8G8R8_UNORM : return 4;
        case VK_FORMAT_B8G8R8_SNORM : return 4;
        case VK_FORMAT_B8G8R8_USCALED : return 4;
        case VK_FORMAT_B8G8R8_SSCALED : return 4;
        case VK_FORMAT_B8G8R8_UINT : return 4;
        case VK_FORMAT_B8G8R8_SINT : return 4;
        case VK_FORMAT_B8G8R8_SRGB : return 4;
        case VK_FORMAT_R8G8B8A8_UNORM : return 4;
        case VK_FORMAT_R8G8B8A8_SNORM : return 4;
        case VK_FORMAT_R8G8B8A8_USCALED : return 4;
        case VK_FORMAT_R8G8B8A8_SSCALED : return 4;
        case VK_FORMAT_R8G8B8A8_UINT : return 4;
        case VK_FORMAT_R8G8B8A8_SINT : return 4;
        case VK_FORMAT_R8G8B8A8_SRGB : return 4;
        case VK_FORMAT_B8G8R8A8_UNORM : return 4;
        case VK_FORMAT_B8G8R8A8_SNORM : return 4;
        case VK_FORMAT_B8G8R8A8_USCALED : return 4;
        case VK_FORMAT_B8G8R8A8_SSCALED : return 4;
        case VK_FORMAT_B8G8R8A8_UINT : return 4;
        case VK_FORMAT_B8G8R8A8_SINT : return 4;
        case VK_FORMAT_B8G8R8A8_SRGB : return 4;
        case VK_FORMAT_A8B8G8R8_UNORM_PACK32 : return 4;
        case VK_FORMAT_A8B8G8R8_SNORM_PACK32 : return 4;
        case VK_FORMAT_A8B8G8R8_USCALED_PACK32 : return 4;
        case VK_FORMAT_A8B8G8R8_SSCALED_PACK32 : return 4;
        case VK_FORMAT_A8B8G8R8_UINT_PACK32 : return 4;
        case VK_FORMAT_A8B8G8R8_SINT_PACK32 : return 4;
        case VK_FORMAT_A8B8G8R8_SRGB_PACK32 : return 4;
        case VK_FORMAT_A2R10G10B10_UNORM_PACK32 : return 4;
        case VK_FORMAT_A2R10G10B10_SNORM_PACK32 : return 4;
        case VK_FORMAT_A2R10G10B10_USCALED_PACK32 : return 4;
        case VK_FORMAT_A2R10G10B10_SSCALED_PACK32 : return 4;
        case VK_FORMAT_A2R10G10B10_UINT_PACK32 : return 4;
        case VK_FORMAT_A2R10G10B10_SINT_PACK32 : return 4;
        case VK_FORMAT_A2B10G10R10_UNORM_PACK32 : return 4;
        case VK_FORMAT_A2B10G10R10_SNORM_PACK32 : return 4;
        case VK_FORMAT_A2B10G10R10_USCALED_PACK32 : return 4;
        case VK_FORMAT_A2B10G10R10_SSCALED_PACK32 : return 4;
        case VK_FORMAT_A2B10G10R10_UINT_PACK32 : return 4;
        case VK_FORMAT_A2B10G10R10_SINT_PACK32 : return 4;
        case VK_FORMAT_R16_UNORM : return 4;
        case VK_FORMAT_R16_SNORM : return 4;
        case VK_FORMAT_R16_USCALED : return 4;
        case VK_FORMAT_R16_SSCALED : return 4;
        case VK_FORMAT_R16_UINT : return 4;
        case VK_FORMAT_R16_SINT : return 4;
        case VK_FORMAT_R16_SFLOAT : return 4;
        case VK_FORMAT_R16G16_UNORM : return 4;
        case VK_FORMAT_R16G16_SNORM : return 4;
        case VK_FORMAT_R16G16_USCALED : return 4;
        case VK_FORMAT_R16G16_SSCALED : return 4;
        case VK_FORMAT_R16G16_UINT : return 4;
        case VK_FORMAT_R16G16_SINT : return 4;
        case VK_FORMAT_R16G16_SFLOAT : return 4;
        case VK_FORMAT_R16G16B16_UNORM : return 4;
        case VK_FORMAT_R16G16B16_SNORM : return 4;
        case VK_FORMAT_R16G16B16_USCALED : return 4;
        case VK_FORMAT_R16G16B16_SSCALED : return 4;
        case VK_FORMAT_R16G16B16_UINT : return 4;
        case VK_FORMAT_R16G16B16_SINT : return 4;
        case VK_FORMAT_R16G16B16_SFLOAT : return 4;
        case VK_FORMAT_R16G16B16A16_UNORM : return 4;
        case VK_FORMAT_R16G16B16A16_SNORM : return 4;
        case VK_FORMAT_R16G16B16A16_USCALED : return 4;
        case VK_FORMAT_R16G16B16A16_SSCALED : return 4;
        case VK_FORMAT_R16G16B16A16_UINT : return 4;
        case VK_FORMAT_R16G16B16A16_SINT : return 4;
        case VK_FORMAT_R16G16B16A16_SFLOAT : return 4;
        case VK_FORMAT_R32_UINT : return 4;
        case VK_FORMAT_R32_SINT : return 4;
        case VK_FORMAT_R32_SFLOAT : return 4;
        case VK_FORMAT_R32G32_UINT : return 4;
        case VK_FORMAT_R32G32_SINT : return 4;
        case VK_FORMAT_R32G32_SFLOAT : return 4;
        case VK_FORMAT_R32G32B32_UINT : return 4;
        case VK_FORMAT_R32G32B32_SINT : return 4;
        case VK_FORMAT_R32G32B32_SFLOAT : return 4;
        case VK_FORMAT_R32G32B32A32_UINT : return 4;
        case VK_FORMAT_R32G32B32A32_SINT : return 4;
        case VK_FORMAT_R32G32B32A32_SFLOAT : return 4;
        case VK_FORMAT_R64_UINT : return 4;
        case VK_FORMAT_R64_SINT : return 4;
        case VK_FORMAT_R64_SFLOAT : return 4;
        case VK_FORMAT_R64G64_UINT : return 4;
        case VK_FORMAT_R64G64_SINT : return 4;
        case VK_FORMAT_R64G64_SFLOAT : return 4;
        case VK_FORMAT_R64G64B64_UINT : return 4;
        case VK_FORMAT_R64G64B64_SINT : return 4;
        case VK_FORMAT_R64G64B64_SFLOAT : return 4;
        case VK_FORMAT_R64G64B64A64_UINT : return 4;
        case VK_FORMAT_R64G64B64A64_SINT : return 4;
        case VK_FORMAT_R64G64B64A64_SFLOAT : return 4;
        case VK_FORMAT_B10G11R11_UFLOAT_PACK32 : return 4;
        case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32 : return 4;
        case VK_FORMAT_D16_UNORM : return 4;
        case VK_FORMAT_X8_D24_UNORM_PACK32 : return 4;
        case VK_FORMAT_D32_SFLOAT : return 4;
        case VK_FORMAT_S8_UINT : return 4;
        case VK_FORMAT_D16_UNORM_S8_UINT : return 4;
        case VK_FORMAT_D24_UNORM_S8_UINT : return 4;
        case VK_FORMAT_D32_SFLOAT_S8_UINT : return 4;
        case VK_FORMAT_BC1_RGB_UNORM_BLOCK : return 4;
        case VK_FORMAT_BC1_RGB_SRGB_BLOCK : return 4;
        case VK_FORMAT_BC1_RGBA_UNORM_BLOCK : return 4;
        case VK_FORMAT_BC1_RGBA_SRGB_BLOCK : return 4;
        case VK_FORMAT_BC2_UNORM_BLOCK : return 4;
        case VK_FORMAT_BC2_SRGB_BLOCK : return 4;
        case VK_FORMAT_BC3_UNORM_BLOCK : return 4;
        case VK_FORMAT_BC3_SRGB_BLOCK : return 4;
        case VK_FORMAT_BC4_UNORM_BLOCK : return 4;
        case VK_FORMAT_BC4_SNORM_BLOCK : return 4;
        case VK_FORMAT_BC5_UNORM_BLOCK : return 4;
        case VK_FORMAT_BC5_SNORM_BLOCK : return 4;
        case VK_FORMAT_BC6H_UFLOAT_BLOCK : return 4;
        case VK_FORMAT_BC6H_SFLOAT_BLOCK : return 4;
        case VK_FORMAT_BC7_UNORM_BLOCK : return 4;
        case VK_FORMAT_BC7_SRGB_BLOCK : return 4;
        case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK : return 4;
        case VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK : return 4;
        case VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK : return 4;
        case VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK : return 4;
        case VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK : return 4;
        case VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK : return 4;
        case VK_FORMAT_EAC_R11_UNORM_BLOCK : return 4;
        case VK_FORMAT_EAC_R11_SNORM_BLOCK : return 4;
        case VK_FORMAT_EAC_R11G11_UNORM_BLOCK : return 4;
        case VK_FORMAT_EAC_R11G11_SNORM_BLOCK : return 4;
        case VK_FORMAT_ASTC_4x4_UNORM_BLOCK : return 4;
        case VK_FORMAT_ASTC_4x4_SRGB_BLOCK : return 4;
        case VK_FORMAT_ASTC_5x4_UNORM_BLOCK : return 4;
        case VK_FORMAT_ASTC_5x4_SRGB_BLOCK : return 4;
        case VK_FORMAT_ASTC_5x5_UNORM_BLOCK : return 4;
        case VK_FORMAT_ASTC_5x5_SRGB_BLOCK : return 4;
        case VK_FORMAT_ASTC_6x5_UNORM_BLOCK : return 4;
        case VK_FORMAT_ASTC_6x5_SRGB_BLOCK : return 4;
        case VK_FORMAT_ASTC_6x6_UNORM_BLOCK : return 4;
        case VK_FORMAT_ASTC_6x6_SRGB_BLOCK : return 4;
        case VK_FORMAT_ASTC_8x5_UNORM_BLOCK : return 4;
        case VK_FORMAT_ASTC_8x5_SRGB_BLOCK : return 4;
        case VK_FORMAT_ASTC_8x6_UNORM_BLOCK : return 4;
        case VK_FORMAT_ASTC_8x6_SRGB_BLOCK : return 4;
        case VK_FORMAT_ASTC_8x8_UNORM_BLOCK : return 4;
        case VK_FORMAT_ASTC_8x8_SRGB_BLOCK : return 4;
        case VK_FORMAT_ASTC_10x5_UNORM_BLOCK : return 4;
        case VK_FORMAT_ASTC_10x5_SRGB_BLOCK : return 4;
        case VK_FORMAT_ASTC_10x6_UNORM_BLOCK : return 4;
        case VK_FORMAT_ASTC_10x6_SRGB_BLOCK : return 4;
        case VK_FORMAT_ASTC_10x8_UNORM_BLOCK : return 4;
        case VK_FORMAT_ASTC_10x8_SRGB_BLOCK : return 4;
        case VK_FORMAT_ASTC_10x10_UNORM_BLOCK : return 4;
        case VK_FORMAT_ASTC_10x10_SRGB_BLOCK : return 4;
        case VK_FORMAT_ASTC_12x10_UNORM_BLOCK : return 4;
        case VK_FORMAT_ASTC_12x10_SRGB_BLOCK : return 4;
        case VK_FORMAT_ASTC_12x12_UNORM_BLOCK : return 4;
        case VK_FORMAT_ASTC_12x12_SRGB_BLOCK : return 4;
        case VK_FORMAT_G8B8G8R8_422_UNORM : return 4;
        case VK_FORMAT_B8G8R8G8_422_UNORM : return 4;
        case VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM : return 4;
        case VK_FORMAT_G8_B8R8_2PLANE_420_UNORM : return 4;
        case VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM : return 4;
        case VK_FORMAT_G8_B8R8_2PLANE_422_UNORM : return 4;
        case VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM : return 4;
        case VK_FORMAT_R10X6_UNORM_PACK16 : return 4;
        case VK_FORMAT_R10X6G10X6_UNORM_2PACK16 : return 4;
        case VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16 : return 4;
        case VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16 : return 4;
        case VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16 : return 4;
        case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16 : return 4;
        case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16 : return 4;
        case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16 : return 4;
        case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16 : return 4;
        case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16 : return 4;
        case VK_FORMAT_R12X4_UNORM_PACK16 : return 4;
        case VK_FORMAT_R12X4G12X4_UNORM_2PACK16 : return 4;
        case VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16 : return 4;
        case VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16 : return 4;
        case VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16 : return 4;
        case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16 : return 4;
        case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16 : return 4;
        case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16 : return 4;
        case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16 : return 4;
        case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16 : return 4;
        case VK_FORMAT_G16B16G16R16_422_UNORM : return 4;
        case VK_FORMAT_B16G16R16G16_422_UNORM : return 4;
        case VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM : return 4;
        case VK_FORMAT_G16_B16R16_2PLANE_420_UNORM : return 4;
        case VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM : return 4;
        case VK_FORMAT_G16_B16R16_2PLANE_422_UNORM : return 4;
        case VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM : return 4;
        case VK_FORMAT_G8_B8R8_2PLANE_444_UNORM : return 4;
        case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16 : return 4;
        case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16 : return 4;
        case VK_FORMAT_G16_B16R16_2PLANE_444_UNORM : return 4;
        case VK_FORMAT_A4R4G4B4_UNORM_PACK16 : return 4;
        case VK_FORMAT_A4B4G4R4_UNORM_PACK16 : return 4;
        case VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK : return 4;
        case VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK : return 4;
        case VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK : return 4;
        case VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK : return 4;
        case VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK : return 4;
        case VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK : return 4;
        case VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK : return 4;
        case VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK : return 4;
        case VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK : return 4;
        case VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK : return 4;
        case VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK : return 4;
        case VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK : return 4;
        case VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK : return 4;
        case VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK : return 4;
        case VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG : return 4;
        case VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG : return 4;
        case VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG : return 4;
        case VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG : return 4;
        case VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG : return 4;
        case VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG : return 4;
        case VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG : return 4;
        case VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG : return 4;
        case VK_FORMAT_R16G16_SFIXED5_NV : return 4;
        case VK_FORMAT_A1B5G5R5_UNORM_PACK16_KHR : return 4;
        case VK_FORMAT_A8_UNORM_KHR : return 4;
        default: return 4;
    }
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