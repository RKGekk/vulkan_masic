#include "vulkan_image_buffer.h"

#include "vulkan_device.h"
#include "vulkan_buffer.h"

bool VulkanImageBuffer::init(std::shared_ptr<VulkanDevice> device, unsigned char* pixels, VkImageCreateInfo image_info, VkImageAspectFlags aspect_flags) {
    size_t bytes_count = VulkanDevice::getBytesCount(image_info.format);
    uint32_t mip_levels = static_cast<uint32_t>(std::floor(std::log2(std::max(image_info.extent.width, image_info.extent.height)))) + 1u;

    m_device = std::move(device);
    m_image_info = image_info;
    m_layout = image_info.initialLayout;
    
    VkResult result = vkCreateImage(m_device->getDevice(), &image_info, nullptr, &m_image);
    if(result != VK_SUCCESS) {
        throw std::runtime_error("failed to create image!");
    }
    
    VkMemoryRequirements mem_req{};
    vkGetImageMemoryRequirements(m_device->getDevice(), m_image, &mem_req);
    m_image_size = mem_req.size;
    
    uint32_t mem_type_idx = device->findMemoryType(mem_req.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
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

    if(!pixels) {
        return true;
    }

    VulkanBuffer staging_buffer;
    staging_buffer.init(device, nullptr, m_image_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    
    void* data;
    vkMapMemory(m_device->getDevice(), staging_buffer.getMemory(), 0, m_image_size, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(m_image_size));
    vkUnmapMemory(m_device->getDevice(), staging_buffer.getMemory());
    
    CommandBatch command_buffer = m_device->getCommandManager().allocCommandBuffer(PoolTypeEnum::TRANSFER);
    m_device->getCommandManager().transitionImageLayout(command_buffer.getCommandBufer(), m_image, image_info.format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mip_levels);
    m_device->getCommandManager().copyBufferToImage(command_buffer.getCommandBufer(), staging_buffer.getBuffer(), m_image, static_cast<uint32_t>(image_info.extent.width), static_cast<uint32_t>(image_info.extent.height));
    m_device->getCommandManager().generateMipmaps(command_buffer.getCommandBufer(), m_image, image_info.format, image_info.extent.width, image_info.extent.height, mip_levels);
    //m_device->getCommandManager().transitionImageLayout(m_image, image_info.format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, mip_levels);
    m_device->getCommandManager().submitCommandBuffer(command_buffer);
    m_device->getCommandManager().wait(PoolTypeEnum::TRANSFER);

    staging_buffer.destroy();

    m_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    m_image_view = device->createImageView(m_image, m_image_info.format, aspect_flags, m_image_info.mipLevels);

    return true;
}

bool VulkanImageBuffer::init(std::shared_ptr<VulkanDevice> device, unsigned char* pixels, size_t width, size_t height, VkFormat format, VkImageAspectFlags aspect_flags) {
    uint32_t mip_levels = static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1u;

    VkFormat image_format = device->findSupportedFormat(
        {
            VK_FORMAT_R8G8B8A8_SRGB
        },
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT
    );
    
    std::vector<uint32_t> families = device->getCommandManager().getQueueFamilyIndices().getIndices();
    VkImageCreateInfo image_info;
    image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.extent.width = static_cast<uint32_t>(width);
    image_info.extent.height = static_cast<uint32_t>(height);
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

    return init(std::move(device), pixels, image_info, aspect_flags);
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