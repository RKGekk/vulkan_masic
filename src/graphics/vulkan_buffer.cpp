#include "vulkan_buffer.h"

#include "vulkan_device.h"
#include "vulkan_command_buffer.h"


bool VulkanBuffer::init(std::shared_ptr<VulkanDevice> device, const void* data, VkDeviceSize buffer_size, VkMemoryPropertyFlags properties, VkBufferUsageFlags usage) {
    m_device = std::move(device);
    m_size = buffer_size;
    m_usage = usage;
    m_properties = properties;

    if (!buffer_size) return true;
    
    std::vector<uint32_t> family_indices = m_device->getCommandManager().getQueueFamilyIndices().getIndices();
    
    VkBufferCreateInfo buffer_info{};
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size = buffer_size;
    buffer_info.usage = data || (m_properties & (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) ? VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage : usage;
    buffer_info.sharingMode = m_device->getCommandManager().getBufferSharingMode();
    buffer_info.queueFamilyIndexCount = static_cast<uint32_t>(family_indices.size());
    buffer_info.pQueueFamilyIndices = family_indices.data();
    
    VkResult result = vkCreateBuffer(m_device->getDevice(), &buffer_info, nullptr, &m_buffer);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to create buffer!");
    }
     
    VkMemoryRequirements mem_req;
    vkGetBufferMemoryRequirements(m_device->getDevice(), m_buffer, &mem_req);
    uint32_t mem_type_idx = m_device->findMemoryType(mem_req.memoryTypeBits, properties);
  
    VkMemoryAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = mem_req.size;
    alloc_info.memoryTypeIndex = mem_type_idx;
 
    result = vkAllocateMemory(m_device->getDevice(), &alloc_info, nullptr, &m_memory);
    if(result != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate buffer memory!");
    }
    VkDeviceSize offset = 0u;
    vkBindBufferMemory(m_device->getDevice(), m_buffer, m_memory, offset);

    if(m_properties & (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
        vkMapMemory(m_device->getDevice(), m_memory, 0, m_size, 0u, &m_mapped);
    }
    
    if(data) {
        update(data, buffer_size);
    }
 
    return true;
}

bool VulkanBuffer::init(std::shared_ptr<VulkanDevice> device, CommandBatch& command_buffer, const void* data, VkDeviceSize buffer_size, VkMemoryPropertyFlags properties, VkBufferUsageFlags usage) {
    init(device, nullptr, buffer_size, properties, usage);
    update(command_buffer, data, buffer_size);
 
    return true;
}

void VulkanBuffer::destroy() {
    if(m_size) {
        if(m_properties & (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
            vkUnmapMemory(m_device->getDevice(), m_memory);
        }
        vkDestroyBuffer(m_device->getDevice(), m_buffer, nullptr);
        vkFreeMemory(m_device->getDevice(), m_memory, nullptr);
    }
}

VkBuffer VulkanBuffer::getBuffer() const {
    return m_buffer;
}

VkDeviceMemory VulkanBuffer::getMemory() const {
    return m_memory;
}

void* VulkanBuffer::getMappedBuffer() const {
    if (m_properties & (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
        return m_mapped;
    }
    throw std::runtime_error("failed map device buffer to host!");
}

VkDeviceSize VulkanBuffer::getSize() const {
    return m_size;
}

VkMemoryPropertyFlags VulkanBuffer::getProperties() const {
    return m_properties;
}

VkBufferUsageFlags VulkanBuffer::getUsage() const {
    return m_usage;
}

VkBufferView VulkanBuffer::createBufferView(VkFormat format, VkDeviceSize range, VkDeviceSize offset) const {
    VkBufferView view;

    VkBufferViewCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
    create_info.pNext = nullptr;
    create_info.flags = 0u;
    create_info.buffer = m_buffer;
    create_info.format = format;
    create_info.offset = offset;
    create_info.range = range;

    VkResult result = vkCreateBufferView(m_device->getDevice(), &create_info, nullptr, &view);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to create buffer view!");
    }
    
    return view;
}

void VulkanBuffer::update(const void* src_data, VkDeviceSize buffer_size) {
    if (buffer_size > m_size) {
        destroy();
        init(m_device, src_data, buffer_size, m_properties, m_usage);
        return;
    }
    if(!src_data) {
        return;
    }
    if(m_properties & (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
        memcpy(m_mapped, src_data, buffer_size);
        return;
    }
    else {
        CommandBatch command_buffer = m_device->getCommandManager().allocCommandBuffer(PoolTypeEnum::TRANSFER);
        update(command_buffer, src_data, buffer_size);
        m_device->getCommandManager().submitCommandBuffer(command_buffer);
        m_device->getCommandManager().wait(PoolTypeEnum::TRANSFER);
        return;
    }
}

void VulkanBuffer::update(CommandBatch& command_buffer, const void* src_data, VkDeviceSize buffer_size, VkAccessFlags dstAccessMask) {
    if (buffer_size > m_size) {
        destroy();
        init(m_device, src_data, buffer_size, m_properties, m_usage);
        return;
    }
    if(!src_data) {
        return;
    }
    if(m_properties & (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
        memcpy(m_mapped, src_data, buffer_size);
        return;
    }
    std::shared_ptr<VulkanBuffer> staging_buffer = std::make_shared<VulkanBuffer>();
    staging_buffer->init(m_device, src_data, m_size, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    command_buffer.addResource(staging_buffer);

    m_device->getCommandManager().copyBuffer(command_buffer.getCommandBufer(), staging_buffer->getBuffer(), m_buffer, buffer_size);

    VkMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = dstAccessMask;

    vkCmdPipelineBarrier(command_buffer.getCommandBufer(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0u, 1u, &barrier, 0u, nullptr, 0u, nullptr);
}

void VulkanBuffer::update(CommandBatch& command_buffer, const void* src_data, VkDeviceSize buffer_size) {
    update(command_buffer, src_data, buffer_size, VulkanDevice::getDstAccessMask(m_usage));
}