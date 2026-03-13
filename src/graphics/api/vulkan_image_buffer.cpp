#include "vulkan_image_buffer.h"

#include "vulkan_device.h"
#include "vulkan_buffer.h"
#include "../pod/format_config.h"
#include "../pod/image_buffer_config.h"
#include "vulkan_resources_manager.h"
#include "../../application.h"

#include <stb_image.h>

VulkanImageBuffer::VulkanImageBuffer(std::shared_ptr<VulkanDevice> device, std::string name) : m_device(std::move(device)), m_name(std::move(name)) {}
VulkanImageBuffer::VulkanImageBuffer(std::shared_ptr<VulkanDevice> device) : m_device(std::move(device)), m_name(std::to_string(rand())) {}


bool VulkanImageBuffer::init(VkImage image, std::shared_ptr<ImageBufferConfig> image_buffer_config) {
    m_image = image;
    m_image_config = std::move(image_buffer_config);

    VkMemoryRequirements mem_req{};
    vkGetImageMemoryRequirements(m_device->getDevice(), m_image, &mem_req);
    m_image_size = mem_req.size;

    for(const auto&[view_type_name, view_cfg_ptr] : m_image_config->getViewInfoMap()) {
        VkResult result = vkCreateImageView(m_device->getDevice(), &view_cfg_ptr->image_view_info, nullptr, &m_image_view_map[view_type_name]);
        if(result != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture image view!");
        }
    }

    return true;
}

bool VulkanImageBuffer::init(unsigned char* pixels, std::shared_ptr<ImageBufferConfig> image_buffer_config) {
    using namespace std::literals;

    m_image_config = std::move(image_buffer_config);
    
    VkResult result = vkCreateImage(m_device->getDevice(), &m_image_config->getImageInfo(), nullptr, &m_image);
    if(result != VK_SUCCESS) {
        throw std::runtime_error("failed to create image!");
    }
    
    VkMemoryRequirements mem_req{};
    vkGetImageMemoryRequirements(m_device->getDevice(), m_image, &mem_req);
    m_image_size = mem_req.size;
    
    uint32_t mem_type_idx = m_device->findMemoryType(mem_req.memoryTypeBits, m_image_config->getMemoryProperties());
    VkMemoryAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = mem_req.size;
    alloc_info.memoryTypeIndex = mem_type_idx;
    
    result = vkAllocateMemory(m_device->getDevice(), &alloc_info, nullptr, &m_memory);
    if(result != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate image memory!");
    }

    VkDeviceSize offset = 0u;
    vkBindImageMemory(m_device->getDevice(), m_image, m_memory, offset);

    for(const auto&[view_type_name, view_cfg_ptr] : m_image_config->getViewInfoMap()) {
        VkResult result = vkCreateImageView(m_device->getDevice(), &view_cfg_ptr->image_view_info, nullptr, &m_image_view_map[view_type_name]);
        if(result != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture image view!");
        }
    }

    if(!pixels) {
        return true;
    }

    std::shared_ptr<VulkanBuffer> staging_buffer = Application::Get().GetRenderer().getManagers()->resources_manager->create_buffer(pixels, m_image_size, "basic_staging_buffer"s);
    CommandBatch command_buffer = m_device->getCommandManager()->allocCommandBuffer(PoolTypeEnum::TRANSFER);
    command_buffer.addResource(staging_buffer);

    if(m_image_config->getImageInfo().initialLayout != VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        m_device->getCommandManager()->transitionImageLayout(
            command_buffer.getCommandBufer(),
            m_image,
            m_image_config->getImageInfo().format,
            m_image_config->getImageInfo().initialLayout,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            m_image_config->getImageInfo().mipLevels
        );
    }
    m_device->getCommandManager()->copyBufferToImage(
        command_buffer.getCommandBufer(),
        staging_buffer->getBuffer(),
        m_image,
        m_image_config->getImageInfo().extent,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
    );

    if(m_image_config->getImageInfo().mipLevels != 1u) {
        m_device->getCommandManager()->generateMipmaps(
            command_buffer.getCommandBufer(),
            m_image,
            m_image_config->getImageInfo().format,
            m_image_config->getFormat()->getExtent2D(),
            m_image_config->getImageInfo().mipLevels,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            m_image_config->getAfterInitLayout()
        );
    }
    m_device->getCommandManager()->submitCommandBuffer(command_buffer);
    m_device->getCommandManager()->wait(PoolTypeEnum::TRANSFER);

    return true;
}

bool VulkanImageBuffer::init(const std::shared_ptr<ImageBufferConfig>& image_buffer_config_template, const std::string& path_to_file) {
    using namespace std::literals;

    int tex_width;
    int tex_height;
    int tex_channels;
    stbi_uc* pixels = stbi_load(path_to_file.c_str(), &tex_width, &tex_height, &tex_channels, STBI_rgb_alpha);
    if (!pixels) {
        throw std::runtime_error("failed to load texture image!");
    }

    std::shared_ptr<ImageBufferConfig> image_buffer_config = image_buffer_config_template->makeInstance(path_to_file + "_cfg"s, {(uint32_t)tex_width, (uint32_t)tex_height});
    bool result = init(pixels, std::move(image_buffer_config));

    stbi_image_free(pixels);

    return result;
}

bool VulkanImageBuffer::init(const std::shared_ptr<ImageBufferConfig>& image_buffer_config_template) {
    using namespace std::literals;

    std::shared_ptr<ImageBufferConfig> image_buffer_config = image_buffer_config_template->makeInstance(m_name + "_cfg"s);
    bool result = init((unsigned char*)nullptr, std::move(image_buffer_config));

    return result;
}

bool VulkanImageBuffer::init(CommandBatch& command_buffer, unsigned char* pixels, std::shared_ptr<ImageBufferConfig> image_buffer_config) {
    m_image_config = std::move(image_buffer_config);
    
    VkResult result = vkCreateImage(m_device->getDevice(), &m_image_config->getImageInfo(), nullptr, &m_image);
    if(result != VK_SUCCESS) {
        throw std::runtime_error("failed to create image!");
    }
    
    VkMemoryRequirements mem_req{};
    vkGetImageMemoryRequirements(m_device->getDevice(), m_image, &mem_req);
    m_image_size = mem_req.size;
    
    uint32_t mem_type_idx = m_device->findMemoryType(mem_req.memoryTypeBits, m_image_config->getMemoryProperties());
    VkMemoryAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = mem_req.size;
    alloc_info.memoryTypeIndex = mem_type_idx;
    
    result = vkAllocateMemory(m_device->getDevice(), &alloc_info, nullptr, &m_memory);
    if(result != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate image memory!");
    }

    VkDeviceSize offset = 0u;
    vkBindImageMemory(m_device->getDevice(), m_image, m_memory, offset);

    for(const auto&[view_type_name, view_cfg_ptr] : m_image_config->getViewInfoMap()) {
        VkResult result = vkCreateImageView(m_device->getDevice(), &view_cfg_ptr->image_view_info, nullptr, &m_image_view_map[view_type_name]);
        if(result != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture image view!");
        }
    }

    if(!pixels) {
        return true;
    }

    std::shared_ptr<VulkanBuffer> staging_buffer = Application::Get().GetRenderer().getManagers()->resources_manager->create_buffer(pixels, m_image_size, "basic_staging_buffer"s);
    command_buffer.addResource(staging_buffer);

    if(m_image_config->getImageInfo().initialLayout != VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        m_device->getCommandManager()->transitionImageLayout(
            command_buffer.getCommandBufer(),
            m_image,
            m_image_config->getImageInfo().format,
            m_image_config->getImageInfo().initialLayout,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            m_image_config->getImageInfo().mipLevels
        );
    }
    m_device->getCommandManager()->copyBufferToImage(
        command_buffer.getCommandBufer(),
        staging_buffer->getBuffer(),
        m_image,
        m_image_config->getImageInfo().extent,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
    );

    if(m_image_config->getImageInfo().mipLevels != 1u) {
        m_device->getCommandManager()->generateMipmaps(
            command_buffer.getCommandBufer(),
            m_image,
            m_image_config->getImageInfo().format,
            m_image_config->getFormat()->getExtent2D(),
            m_image_config->getImageInfo().mipLevels,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            m_image_config->getAfterInitLayout()
        );
    }
    m_device->getCommandManager()->submitCommandBuffer(command_buffer);
    m_device->getCommandManager()->wait(PoolTypeEnum::TRANSFER);

    return true;
}

bool VulkanImageBuffer::init(CommandBatch& command_buffer, const std::shared_ptr<ImageBufferConfig>& image_buffer_config_template, const std::string& path_to_file) {
    using namespace std::literals;

    int tex_width;
    int tex_height;
    int tex_channels;
    stbi_uc* pixels = stbi_load(path_to_file.c_str(), &tex_width, &tex_height, &tex_channels, STBI_rgb_alpha);
    if (!pixels) {
        throw std::runtime_error("failed to load texture image!");
    }

    std::shared_ptr<ImageBufferConfig> image_buffer_config = image_buffer_config_template->makeInstance(path_to_file + "_cfg"s, {(uint32_t)tex_width, (uint32_t)tex_height});
    bool result = init(command_buffer, pixels, std::move(image_buffer_config));

    stbi_image_free(pixels);

    return result;
}

void VulkanImageBuffer::destroy() {
}

VkImage VulkanImageBuffer::getImageBuffer() const {
    return m_image;
}
    
VkImageView VulkanImageBuffer::getImageBufferView() const {
    return (*m_image_view_map.begin()).second;
};

VkImageView VulkanImageBuffer::getImageBufferView(const std::string& view_name) const {
    return m_image_view_map.at(view_name);
}

const std::unordered_map<std::string, VkImageView>& VulkanImageBuffer::getImageViewMap() const {
    return m_image_view_map;
}


VkDeviceMemory VulkanImageBuffer::getMemory() const {
    return m_memory;
}

VkDeviceSize VulkanImageBuffer::getSize() const {
    return m_image_size;
}

VkSubresourceLayout VulkanImageBuffer::getSubresourceSizes(uint32_t mip_level, uint32_t array_layer) const {
    return getSubresourceSizes( (*m_image_config->getViewInfoMap().begin()).second->image_view_info.subresourceRange.aspectMask, mip_level, array_layer);
}

VkSubresourceLayout VulkanImageBuffer::getSubresourceSizes(VkImageAspectFlags aspect, uint32_t mip_level, uint32_t array_layer) const {
    VkSubresourceLayout result{};
    VkImageSubresource subresource{};
    subresource.aspectMask = aspect;
    subresource.mipLevel = mip_level;
    subresource.arrayLayer = array_layer;

    vkGetImageSubresourceLayout(m_device->getDevice(), m_image, &subresource, &result);

    return result;
}

const std::shared_ptr<ImageBufferConfig>& VulkanImageBuffer::getImageConfig() const {
    return m_image_config;
}

void VulkanImageBuffer::changeLayout(VkImageLayout old_layout, VkImageLayout new_layout) {
    CommandBatch command_buffer = m_device->getCommandManager()->allocCommandBuffer(PoolTypeEnum::TRANSFER);
    changeLayout(command_buffer, old_layout, new_layout);
}

void VulkanImageBuffer::changeLayout(CommandBatch& command_buffer, VkImageLayout old_layout, VkImageLayout new_layout) {
    m_device->getCommandManager()->transitionImageLayout(
        command_buffer.getCommandBufer(),
        m_image,
        m_image_config->getImageInfo().format,
        old_layout,
        new_layout,
        m_image_config->getImageInfo().mipLevels
    );
    m_device->getCommandManager()->submitCommandBuffer(command_buffer);
    m_device->getCommandManager()->wait(PoolTypeEnum::TRANSFER);
}

const RenderResource::ResourceName& VulkanImageBuffer::getName() const {
    return m_name;
}

RenderResource::Type VulkanImageBuffer::getType() const {
    return RenderResource::Type::IMAGE;
}