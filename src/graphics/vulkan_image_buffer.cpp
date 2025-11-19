#include "vulkan_image_buffer.h"

#include "vulkan_device.h"
#include "vulkan_buffer.h"

bool VulkanImageBuffer::init(std::shared_ptr<VulkanDevice> device, VkImage image, VkExtent2D extent, VkFormat format, VkImageAspectFlags aspect_flags, uint32_t mip_levels) {
    m_device = std::move(device);
    m_image = image;

    VkFormat image_format = m_device->findSupportedFormat(
        {
            format
        },
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT
    );
    
    static std::vector<uint32_t> families = m_device->getCommandManager().getQueueFamilyIndices().getIndices();
    m_image_info = {};
    m_image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    m_image_info.imageType = VK_IMAGE_TYPE_2D;
    m_image_info.extent.width = static_cast<uint32_t>(extent.width);
    m_image_info.extent.height = static_cast<uint32_t>(extent.height);
    m_image_info.extent.depth = 1u;
    m_image_info.mipLevels = mip_levels;
    m_image_info.arrayLayers = 1u;
    m_image_info.format = image_format;
    m_image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    m_image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    m_image_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    m_image_info.sharingMode = m_device->getCommandManager().getBufferSharingMode();
    m_image_info.queueFamilyIndexCount = static_cast<uint32_t>(families.size());
    m_image_info.pQueueFamilyIndices = families.data();
    m_image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    m_image_info.flags = 0u;

    VkMemoryRequirements mem_req{};
    vkGetImageMemoryRequirements(m_device->getDevice(), m_image, &mem_req);
    m_image_size = mem_req.size;

    m_layout = m_image_info.initialLayout;

    m_image_view = createImageView(m_image, m_image_info.format, aspect_flags, m_image_info.mipLevels);    

    return true;
}

bool VulkanImageBuffer::init(std::shared_ptr<VulkanDevice> device, unsigned char* pixels, VkImageCreateInfo image_info, VkMemoryPropertyFlags properties, VkImageAspectFlags aspect_flags) {
    size_t bytes_count = VulkanDevice::getBytesCount(image_info.format);
    size_t initial_image_size = image_info.extent.width * image_info.extent.height * bytes_count;
    uint32_t mip_levels = image_info.mipLevels;

    m_device = std::move(device);
    m_image_info = image_info;
    m_layout = image_info.initialLayout;
    m_properties = properties;
    
    VkResult result = vkCreateImage(m_device->getDevice(), &image_info, nullptr, &m_image);
    if(result != VK_SUCCESS) {
        throw std::runtime_error("failed to create image!");
    }
    
    VkMemoryRequirements mem_req{};
    vkGetImageMemoryRequirements(m_device->getDevice(), m_image, &mem_req);
    m_image_size = mem_req.size;
    
    uint32_t mem_type_idx = m_device->findMemoryType(mem_req.memoryTypeBits, properties);
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

    m_image_view = createImageView(m_image, m_image_info.format, aspect_flags, m_image_info.mipLevels);

    if(!pixels) {
        return true;
    }

    std::shared_ptr<VulkanBuffer> staging_buffer = std::make_shared<VulkanBuffer>();
    staging_buffer->init(m_device, pixels, initial_image_size, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    //staging_buffer->update(pixels, initial_image_size);
    
    CommandBatch command_buffer = m_device->getCommandManager().allocCommandBuffer(PoolTypeEnum::TRANSFER);
    command_buffer.addResource(staging_buffer);
    m_device->getCommandManager().transitionImageLayout(command_buffer.getCommandBufer(), m_image, image_info.format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mip_levels);
    m_device->getCommandManager().copyBufferToImage(command_buffer.getCommandBufer(), staging_buffer->getBuffer(), m_image, static_cast<uint32_t>(image_info.extent.width), static_cast<uint32_t>(image_info.extent.height));
    m_device->getCommandManager().generateMipmaps(command_buffer.getCommandBufer(), m_image, image_info.format, image_info.extent.width, image_info.extent.height, mip_levels);
    //m_device->getCommandManager().transitionImageLayout(m_image, image_info.format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, mip_levels);
    m_device->getCommandManager().submitCommandBuffer(command_buffer);
    m_device->getCommandManager().wait(PoolTypeEnum::TRANSFER);

    m_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    return true;
}

bool VulkanImageBuffer::init(std::shared_ptr<VulkanDevice> device, unsigned char* pixels, VkExtent2D extent, VkFormat format, VkMemoryPropertyFlags properties, VkImageAspectFlags aspect_flags) {
    uint32_t mip_levels = static_cast<uint32_t>(std::floor(std::log2(std::max(extent.width, extent.height))));

    VkFormat image_format = device->findSupportedFormat(
        {
            format
        },
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT
    );
    
    static std::vector<uint32_t> families = device->getCommandManager().getQueueFamilyIndices().getIndices();
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
    image_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    image_info.sharingMode = device->getCommandManager().getBufferSharingMode();
    image_info.queueFamilyIndexCount = static_cast<uint32_t>(families.size());
    image_info.pQueueFamilyIndices = families.data();
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_info.flags = 0u;

    return init(device, pixels, image_info, properties, aspect_flags);
}

void VulkanImageBuffer::destroy() {
    vkDestroyImageView(m_device->getDevice(), m_image_view, nullptr);
    vkDestroyImage(m_device->getDevice(), m_image, nullptr);
    vkFreeMemory(m_device->getDevice(), m_memory, nullptr);
}

VkImage VulkanImageBuffer::getImageBuffer() const {
    return m_image;
}
    
VkImageView VulkanImageBuffer::getImageBufferView() const {
    return m_image_view;
};

VkDeviceMemory VulkanImageBuffer::getMemory() const {
    return m_memory;
}

VkDeviceSize VulkanImageBuffer::getSize() const {
    return m_image_size;
}

VkImageLayout VulkanImageBuffer::getLayout() const {
    return m_layout;
}

const VkImageCreateInfo& VulkanImageBuffer::getImageInfo() const {
    return m_image_info;
}

void VulkanImageBuffer::changeLayout(VkImageLayout new_layout) {
    CommandBatch command_buffer = m_device->getCommandManager().allocCommandBuffer(PoolTypeEnum::TRANSFER);
    changeLayout(command_buffer, new_layout);
}

void VulkanImageBuffer::changeLayout(CommandBatch& command_buffer, VkImageLayout new_layout) {
    m_device->getCommandManager().transitionImageLayout(command_buffer.getCommandBufer(), m_image, m_image_info.format, m_layout, new_layout, m_image_info.mipLevels);
    m_device->getCommandManager().submitCommandBuffer(command_buffer);
    m_device->getCommandManager().wait(PoolTypeEnum::TRANSFER);

    m_layout = new_layout;
}

VkImageView VulkanImageBuffer::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspect_flags, uint32_t mip_levels) const {
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
    VkResult result = vkCreateImageView(m_device->getDevice(), &view_info, nullptr, &image_view);
    if(result != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture image view!");
    }
    
    return image_view;
}