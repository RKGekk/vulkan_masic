#include "vulkan_texture.h"

#include "vulkan_device.h"

//#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <utility>

VulkanTexture::VulkanTexture(std::shared_ptr<VulkanDevice> device, std::string name) : VulkanImageBuffer(std::move(device), std::move(name)) {}

bool VulkanTexture::init(unsigned char* pixels, int width, int height, VkFormat format) {
    std::shared_ptr<VulkanSampler> texture_sampler = std::make_shared<VulkanSampler>();
    texture_sampler->init(m_device, m_image_info.mipLevels);
    return init(pixels, width, height, std::move(texture_sampler), format);
}

bool VulkanTexture::init(unsigned char* pixels, int width, int height, std::shared_ptr<VulkanSampler> sampler, VkFormat format) {
    VulkanImageBuffer::init(pixels, {(uint32_t)width, (uint32_t)height}, format);

    m_texture_sampler = std::move(sampler);

    m_image_desc_info.imageLayout = m_layout;
    m_image_desc_info.imageView = m_image_view;
    m_image_desc_info.sampler = m_texture_sampler->getSampler();

    return true;
}

bool VulkanTexture::init(const std::string& path_to_file) {
    std::shared_ptr<VulkanSampler> texture_sampler = std::make_shared<VulkanSampler>();
    texture_sampler->init(m_device, m_image_info.mipLevels);
    return init(path_to_file, std::move(texture_sampler));
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

bool VulkanTexture::init(const std::string& path_to_file, std::shared_ptr<VulkanSampler> sampler) {
    int tex_width;
    int tex_height;
    int tex_channels;
    stbi_uc* pixels = stbi_load(path_to_file.c_str(), &tex_width, &tex_height, &tex_channels, STBI_rgb_alpha);
    if (!pixels) {
        throw std::runtime_error("failed to load texture image!");
    }

    bool result = init(pixels, tex_width, tex_height, std::move(sampler), VK_FORMAT_R8G8B8A8_UNORM);

    stbi_image_free(pixels);

    return result;
}

bool VulkanTexture::init(unsigned char* data, size_t size) {
    int tex_width;
    int tex_height;
    int tex_channels;
    stbi_uc* pixels = stbi_load_from_memory(data, size, &tex_width, &tex_height, &tex_channels, STBI_rgb_alpha);
    if (!pixels) {
        throw std::runtime_error("failed to load texture image!");
    }
    uint32_t mip_levels = static_cast<uint32_t>(std::floor(std::log2(std::max(tex_width, tex_height))));

    std::shared_ptr<VulkanSampler> texture_sampler = std::make_shared<VulkanSampler>();
    texture_sampler->init(m_device, m_image_info.mipLevels);
    
    bool result = init(pixels, tex_width, tex_height, std::move(texture_sampler), VK_FORMAT_R8G8B8A8_UNORM);

    stbi_image_free(pixels);

    return result;
}

bool VulkanTexture::init(unsigned char* data, size_t size, std::shared_ptr<VulkanSampler> sampler) {
    int tex_width;
    int tex_height;
    int tex_channels;
    stbi_uc* pixels = stbi_load_from_memory(data, size, &tex_width, &tex_height, &tex_channels, STBI_rgb_alpha);
    if (!pixels) {
        throw std::runtime_error("failed to load texture image!");
    }
    uint32_t mip_levels = static_cast<uint32_t>(std::floor(std::log2(std::max(tex_width, tex_height))));

    bool result = init(pixels, tex_width, tex_height, std::move(sampler), VK_FORMAT_R8G8B8A8_UNORM);

    stbi_image_free(pixels);

    return result;
}

void VulkanTexture::destroy() {
    VulkanImageBuffer::destroy();
    m_texture_sampler->destroy();
}

std::shared_ptr<VulkanSampler> VulkanTexture::getSampler() const {
    return m_texture_sampler;
}

VkDescriptorImageInfo VulkanTexture::getDescImageInfo() const {
    return m_image_desc_info;
}