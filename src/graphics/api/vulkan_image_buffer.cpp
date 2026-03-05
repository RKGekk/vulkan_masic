#include "vulkan_image_buffer.h"

#include "vulkan_device.h"
#include "vulkan_buffer.h"

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

    std::shared_ptr<VulkanBuffer> staging_buffer = std::make_shared<VulkanBuffer>(m_device);
    staging_buffer->init(pixels, m_image_size, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    
    CommandBatch command_buffer = m_device->getCommandManager()->allocCommandBuffer(PoolTypeEnum::TRANSFER);
    command_buffer.addResource(staging_buffer);

    m_device->getCommandManager()->copyBufferToImage(command_buffer.getCommandBufer(), staging_buffer->getBuffer(), m_image, m_image_config->getImageInfo().extent, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    if(m_image_config->getImageInfo().mipLevels != 1u) {
        m_device->getCommandManager()->generateMipmaps(
            command_buffer.getCommandBufer(),
            m_image,
            m_image_config->getImageInfo().format,
            m_image_config->getFormat()->getExtent2D(),
            m_image_config->getImageInfo().mipLevels,
            m_image_config->getImageInfo().initialLayout,
            m_image_config->getAfterInitLayout()
        );
    }
    m_device->getCommandManager()->submitCommandBuffer(command_buffer);
    m_device->getCommandManager()->wait(PoolTypeEnum::TRANSFER);

    return true;
}

bool VulkanImageBuffer::init(unsigned char* pixels, VkExtent2D extent, VkFormat format, VkMemoryPropertyFlags properties, VkImageAspectFlags aspect_flags) {
    uint32_t mip_levels = static_cast<uint32_t>(std::floor(std::log2(std::max(extent.width, extent.height))));
    VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
    VkFormat image_format = m_device->findSupportedFormat(
        {
            format
        },
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT,
        usage,
        extent,
        mip_levels,
        samples
    );
    
    static std::vector<uint32_t> families = m_device->getCommandManager()->getQueueFamilyIndices().getIndices();
    VkImageCreateInfo image_info = {};
    image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.extent.width = static_cast<uint32_t>(extent.width);
    image_info.extent.height = static_cast<uint32_t>(extent.height);
    image_info.extent.depth = 1u;
    image_info.mipLevels = mip_levels;
    image_info.arrayLayers = 1u;
    image_info.format = image_format;
    image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_info.usage = usage;
    image_info.sharingMode = m_device->getCommandManager()->getBufferSharingMode();
    image_info.queueFamilyIndexCount = static_cast<uint32_t>(families.size());
    image_info.pQueueFamilyIndices = families.data();
    image_info.samples = samples;
    image_info.flags = 0u;

    return init(pixels, image_info, properties, aspect_flags);
}

void VulkanImageBuffer::destroy() {
}

VkImage VulkanImageBuffer::getImageBuffer() const {
    return m_image;
}
    
VkImageView VulkanImageBuffer::getImageBufferView() const {
    return (*m_image_view_map.begin()).second;
};

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

void VulkanImageBuffer::changeLayout(VkImageLayout new_layout) {
    CommandBatch command_buffer = m_device->getCommandManager()->allocCommandBuffer(PoolTypeEnum::TRANSFER);
    changeLayout(command_buffer, new_layout);
}

void VulkanImageBuffer::changeLayout(CommandBatch& command_buffer, VkImageLayout new_layout) {
    m_device->getCommandManager()->transitionImageLayout(command_buffer.getCommandBufer(), m_image, m_image_info.format, m_layout, new_layout, m_image_info.mipLevels);
    m_device->getCommandManager()->submitCommandBuffer(command_buffer);
    m_device->getCommandManager()->wait(PoolTypeEnum::TRANSFER);

    m_layout = new_layout;
}

const RenderResource::ResourceName& VulkanImageBuffer::getName() const {
    return m_name;
}

RenderResource::Type VulkanImageBuffer::getType() const {
    return RenderResource::Type::IMAGE;
}

VkImageView VulkanImageBuffer::createImageView(VkImageViewCreateInfo view_create_info) const {
    VkImageView image_view;
    VkResult result = vkCreateImageView(m_device->getDevice(), &view_create_info, nullptr, &image_view);
    if(result != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture image view!");
    }
    
    return image_view;
}