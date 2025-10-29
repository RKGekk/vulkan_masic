#include "vulkan_texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

bool VulkanTexture::init(std::shared_ptr<VulkanDevice> device, unsigned char* pixels, int width, int height, VkFormat format) {
    return init(std::move(device), pixels, width, height, createTextureSampler(m_image_info.mipLevels), format);
}

bool VulkanTexture::init(std::shared_ptr<VulkanDevice> device, unsigned char* pixels, int width, int height, VkSampler sampler, VkFormat format) {
    VulkanImageBuffer::init(device, pixels, width, height, format);

    m_texture_sampler = sampler;

    m_image_desc_info.imageLayout = m_layout;
    m_image_desc_info.imageView = m_image_view;
    m_image_desc_info.sampler = m_texture_sampler;

    return true;
}

bool VulkanTexture::init(std::shared_ptr<VulkanDevice> device, const std::string& path_to_file) {
    return init(std::move(device), path_to_file, createTextureSampler(m_image_info.mipLevels));
}

bool VulkanTexture::init(std::shared_ptr<VulkanDevice> device, const std::string& path_to_file, VkSampler sampler) {
    int tex_width;
    int tex_height;
    int tex_channels;
    stbi_uc* pixels = stbi_load(path_to_file.c_str(), &tex_width, &tex_height, &tex_channels, STBI_rgb_alpha);
    if (!pixels) {
        throw std::runtime_error("failed to load texture image!");
    }

    bool result = init(std::move(device), pixels, tex_width, tex_height, sampler);

    stbi_image_free(pixels);

    return result;
}

void VulkanTexture::destroy() {
    VulkanImageBuffer::destroy();
    vkDestroySampler(m_device->getDevice(), m_texture_sampler, nullptr);
}

VkSampler VulkanTexture::getSampler() const {
    return m_texture_sampler;
}

VkDescriptorImageInfo VulkanTexture::getDescImageInfo() const {
    return m_image_desc_info;
}

VkSampler VulkanTexture::createTextureSampler(uint32_t mip_levels) const {
    VkPhysicalDeviceFeatures supported_features{};
    vkGetPhysicalDeviceFeatures(m_device->getDeviceAbilities().physical_device, &supported_features);

    VkPhysicalDeviceProperties device_props{};
    vkGetPhysicalDeviceProperties(m_device->getDeviceAbilities().physical_device, &device_props);
    
    VkSamplerCreateInfo sampler_info{};
    sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler_info.magFilter = VK_FILTER_LINEAR;
    sampler_info.minFilter = VK_FILTER_LINEAR;
    sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.anisotropyEnable = supported_features.samplerAnisotropy;
    sampler_info.maxAnisotropy = device_props.limits.maxSamplerAnisotropy;
    sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    sampler_info.unnormalizedCoordinates = VK_FALSE;
    sampler_info.compareEnable = VK_FALSE;
    sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;
    sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler_info.mipLodBias = 0.0f;
    sampler_info.minLod = 0.0f;
    sampler_info.maxLod = static_cast<float>(mip_levels);;
    
    VkSampler texture_sampler;
    VkResult result = vkCreateSampler(m_device->getDevice(), &sampler_info, nullptr, &texture_sampler);
    if(result != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture sampler!");
    }
    return texture_sampler;
}