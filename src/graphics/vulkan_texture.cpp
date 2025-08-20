#include "vulkan_texture.h"


bool VulkanTexture::init(std::shared_ptr<VulkanDevice> device, const std::string& path_to_file) {
    m_device = std::move(device);

    m_texture_image = m_device->createImageAndView(path_to_file);
    m_texture_sampler = createTextureSampler(m_texture_image.image_info.mipLevels);

    m_image_info = VkDescriptorImageInfo{};
    m_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    m_image_info.imageView = m_texture_image.image_view;
    m_image_info.sampler = m_texture_sampler;

    return true;
}

void VulkanTexture::destroy() {
    vkDestroySampler(m_device->getDevice(), m_texture_sampler, nullptr);
    vkDestroyImageView(m_device->getDevice(), m_texture_image.image_view, nullptr);
    vkDestroyImage(m_device->getDevice(), m_texture_image.image, nullptr);
    vkFreeMemory(m_device->getDevice(), m_texture_image.memory, nullptr);
}

ImageBuffer VulkanTexture::getImageBuffer() const {
    return m_texture_image.getImage();
}

ImageBufferAndView VulkanTexture::getImageBufferAndView() const {
    return m_texture_image;
}

VkSampler VulkanTexture::getSampler() const {
    return m_texture_sampler;
}

VkDeviceSize VulkanTexture::getSize() const {
    return m_texture_image.image_size;
}

VkDescriptorImageInfo VulkanTexture::getDescImageInfo() const {
    return m_image_info;
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