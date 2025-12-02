#include "vulkan_texture.h"

//#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

bool VulkanTexture::init(std::shared_ptr<VulkanDevice> device, unsigned char* pixels, int width, int height, VkFormat format) {
    m_device = device;
    return init(device, pixels, width, height, createTextureSampler(m_image_info.mipLevels), format);
}

bool VulkanTexture::init(std::shared_ptr<VulkanDevice> device, unsigned char* pixels, int width, int height, VkSampler sampler, VkFormat format) {
    VulkanImageBuffer::init(device, pixels, {(uint32_t)width, (uint32_t)height}, format);

    m_texture_sampler = sampler;

    m_image_desc_info.imageLayout = m_layout;
    m_image_desc_info.imageView = m_image_view;
    m_image_desc_info.sampler = m_texture_sampler;

    return true;
}

bool VulkanTexture::init(std::shared_ptr<VulkanDevice> device, const std::string& path_to_file) {
    m_device = device;
    return init(device, path_to_file, createTextureSampler(m_image_info.mipLevels));
}

void fast_unpack(char* rgba, const char* rgb, const int count) {
    if(count==0)
        return;
    for(int i=count; --i; rgba+=4, rgb+=3) {
        *(uint32_t*)(void*)rgba = *(const uint32_t*)(const void*)rgb;
    }
    for(int j=0; j<3; ++j) {
        rgba[j] = rgb[j];
    }
}

bool VulkanTexture::init(std::shared_ptr<VulkanDevice> device, const std::string& path_to_file, VkSampler sampler) {
    int tex_width;
    int tex_height;
    int tex_channels;
    stbi_uc* pixels = stbi_load(path_to_file.c_str(), &tex_width, &tex_height, &tex_channels, STBI_rgb_alpha);
    if (!pixels) {
        throw std::runtime_error("failed to load texture image!");
    }

    bool result = false;
    if(tex_channels == 3) {
        std::vector<uint32_t> mem(tex_width * tex_height);
        fast_unpack(reinterpret_cast<char*>(mem.data()), reinterpret_cast<char*>(pixels), tex_width * tex_height);
        result = init(std::move(device), reinterpret_cast<unsigned char*>(mem.data()), tex_width, tex_height, sampler, VK_FORMAT_R8G8B8A8_UNORM);
    }
    else {
        result = init(std::move(device), pixels, tex_width, tex_height, sampler, VK_FORMAT_R8G8B8A8_UNORM);
    }

    stbi_image_free(pixels);

    return result;
}

bool VulkanTexture::init(std::shared_ptr<VulkanDevice> device, unsigned char* data, size_t size) {
    int tex_width;
    int tex_height;
    int tex_channels;
    stbi_uc* pixels = stbi_load_from_memory(data, size, &tex_width, &tex_height, &tex_channels, STBI_rgb_alpha);
    if (!pixels) {
        throw std::runtime_error("failed to load texture image!");
    }
    uint32_t mip_levels = static_cast<uint32_t>(std::floor(std::log2(std::max(tex_width, tex_height))));

    bool result = false;
    if(tex_channels == 3) {
        std::vector<uint32_t> mem(tex_width * tex_height);
        fast_unpack(reinterpret_cast<char*>(mem.data()), reinterpret_cast<char*>(pixels), tex_width * tex_height);
        result = init(std::move(device), reinterpret_cast<unsigned char*>(mem.data()), tex_width, tex_height, createTextureSampler(mip_levels), VK_FORMAT_R8G8B8A8_UNORM);
    }
    else {
        result = init(std::move(device), pixels, tex_width, tex_height, createTextureSampler(mip_levels), VK_FORMAT_R8G8B8A8_UNORM);
    }

    stbi_image_free(pixels);

    return result;
}

bool VulkanTexture::init(std::shared_ptr<VulkanDevice> device, unsigned char* data, size_t size, VkSampler sampler) {
    int tex_width;
    int tex_height;
    int tex_channels;
    stbi_uc* pixels = stbi_load_from_memory(data, size, &tex_width, &tex_height, &tex_channels, STBI_rgb_alpha);
    if (!pixels) {
        throw std::runtime_error("failed to load texture image!");
    }
    uint32_t mip_levels = static_cast<uint32_t>(std::floor(std::log2(std::max(tex_width, tex_height))));

    bool result = false;
    if(tex_channels == 3) {
        std::vector<uint32_t> mem(tex_width * tex_height);
        fast_unpack(reinterpret_cast<char*>(mem.data()), reinterpret_cast<char*>(pixels), tex_width * tex_height);
        result = init(std::move(device), reinterpret_cast<unsigned char*>(mem.data()), tex_width, tex_height, sampler, VK_FORMAT_R8G8B8A8_UNORM);
    }
    else {
        result = init(std::move(device), pixels, tex_width, tex_height, sampler, VK_FORMAT_R8G8B8A8_UNORM);
    }

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