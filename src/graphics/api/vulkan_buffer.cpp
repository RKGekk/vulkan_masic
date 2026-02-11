#include "vulkan_buffer.h"

#include "vulkan_device.h"
#include "vulkan_command_buffer.h"

VulkanBuffer::VulkanBuffer(std::shared_ptr<VulkanDevice> device, std::string name) : m_device(std::move(device)), m_name(std::move(name)) {}
VulkanBuffer::VulkanBuffer(std::shared_ptr<VulkanDevice> device) : m_device(std::move(device)), m_name(std::to_string(rand())) {};

bool VulkanBuffer::init(const void* data, VkDeviceSize buffer_size, VkMemoryPropertyFlags properties, VkBufferUsageFlags usage) {
    
    m_size = buffer_size;
    m_usage = usage;
    m_properties = properties;

    if (!buffer_size) return true;
    
    std::vector<uint32_t> family_indices = m_device->getCommandManager()->getQueueFamilyIndices().getIndices();
    
    VkBufferCreateInfo buffer_info{};
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size = buffer_size;
    buffer_info.usage = data || (m_properties & (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) ? VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage : usage;
    buffer_info.sharingMode = m_device->getCommandManager()->getBufferSharingMode();
    buffer_info.queueFamilyIndexCount = static_cast<uint32_t>(family_indices.size());
    buffer_info.pQueueFamilyIndices = family_indices.data();
    
    VkResult result = vkCreateBuffer(m_device->getDevice(), &buffer_info, nullptr, &m_buffer);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to create buffer!");
    }
    
    VkMemoryRequirements mem_req;
    vkGetBufferMemoryRequirements(m_device->getDevice(), m_buffer, &mem_req);
    uint32_t mem_type_idx = m_device->findMemoryType(mem_req.memoryTypeBits, properties);
    m_size = mem_req.size;
  
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

bool VulkanBuffer::init(CommandBatch& command_buffer, const void* data, VkDeviceSize buffer_size, VkMemoryPropertyFlags properties, VkBufferUsageFlags usage) {
    init(nullptr, buffer_size, properties, usage);
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
        init(src_data, buffer_size, m_properties, m_usage);
        return;
    }
    if(!src_data) {
        return;
    }
    if(m_properties & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) {
        memcpy(m_mapped, src_data, buffer_size);
        return;
    }
    else if(m_properties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
        memcpy(m_mapped, src_data, buffer_size);
        VkMappedMemoryRange range{};
        range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        range.pNext = nullptr;
        range.memory = m_memory;
        range.offset = 0u;
        range.size = VK_WHOLE_SIZE;
        VkResult result = vkFlushMappedMemoryRanges(m_device->getDevice(), 1u, &range);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to flush buffer to device!");
        }
        return;
    }
    else {
        CommandBatch command_buffer = m_device->getCommandManager()->allocCommandBuffer(PoolTypeEnum::TRANSFER);
        update(command_buffer, src_data, buffer_size);
        m_device->getCommandManager()->submitCommandBuffer(command_buffer);
        m_device->getCommandManager()->wait(PoolTypeEnum::TRANSFER);

        return;
    }
}

void VulkanBuffer::update(CommandBatch& command_buffer, const void* src_data, VkDeviceSize buffer_size, VkAccessFlags dstAccessMask) {
    if (buffer_size > m_size) {
        destroy();
        init(src_data, buffer_size, m_properties, m_usage);
        return;
    }
    if(!src_data) {
        return;
    }
    if(m_properties & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) {
        memcpy(m_mapped, src_data, buffer_size);
        return;
    }
    if(m_properties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
        VkMappedMemoryRange range{};
        range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        range.pNext = nullptr;
        range.memory = m_memory;
        range.offset = 0u;
        range.size = VK_WHOLE_SIZE;

        VkResult result{};
        // send data from GPU
        // result = vkInvalidateMappedMemoryRanges(m_device->getDevice(), 1u, &range);
        // if (result != VK_SUCCESS) {
        //     throw std::runtime_error("failed to invalidate buffer on device to flush data to host!");
        // }

        memcpy(m_mapped, src_data, buffer_size);

        result = vkFlushMappedMemoryRanges(m_device->getDevice(), 1u, &range);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to flush buffer to device!");
        }
        //setGlobalMemoryUpdateBarier(command_buffer, dstAccessMask);
        setMemoryUpdateBarier(command_buffer, dstAccessMask);
        return;
    }

    std::shared_ptr<VulkanBuffer> staging_buffer = std::make_shared<VulkanBuffer>(m_device);
    staging_buffer->init(src_data, m_size, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    command_buffer.addResource(staging_buffer);

    m_device->getCommandManager()->copyBuffer(command_buffer.getCommandBufer(), staging_buffer->getBuffer(), m_buffer, buffer_size);

    // alternative to vkCmdCopyBuffer from m_device->getCommandManager().copyBuffer, the data is consumed from host memory as soon as vkCmdUpdateBuffer() is called, Vulkan make a copy of the data you’ve supplied! Data is not written into the buffer until vkCmdUpdateBuffer() is executed by the device after the command buffer has been submitted! The maximum size of data that can be placed in a buffer with vkCmdUpdateBuffer() is 65,536 bytes.
    // vkCmdUpdateBuffer(command_buffer.getCommandBufer(), m_buffer, 0u, buffer_size, src_data);

    //setGlobalMemoryUpdateBarier(command_buffer, dstAccessMask);
    setMemoryUpdateBarier(command_buffer, dstAccessMask);
}

void VulkanBuffer::update(CommandBatch& command_buffer, const void* src_data, VkDeviceSize buffer_size) {
    update(command_buffer, src_data, buffer_size, VulkanDevice::getDstAccessMask(m_usage));
}

const RenderResource::ResourceName& VulkanBuffer::getName() const {
    return m_name;
}

RenderResource::Type VulkanBuffer::getType() const {
    return RenderResource::Type::UNIFORM_BUFFER;
}

void VulkanBuffer::setGlobalMemoryUpdateBarier(CommandBatch& command_buffer, VkAccessFlags dstAccessMask) {
    VkMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = dstAccessMask;

    vkCmdPipelineBarrier(command_buffer.getCommandBufer(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_DEPENDENCY_BY_REGION_BIT, 1u, &barrier, 0u, nullptr, 0u, nullptr);
}

void VulkanBuffer::setMemoryUpdateBarier(CommandBatch& command_buffer, VkAccessFlags dstAccessMask) {
    VkBufferMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = dstAccessMask;
    //barrier.srcQueueFamilyIndex = command_buffer.getFamilyIndex();
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    //barrier.dstQueueFamilyIndex = command_buffer.getFamilyIndex();
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.buffer = m_buffer;
    barrier.offset = 0u;
    barrier.size = VK_WHOLE_SIZE;

    vkCmdPipelineBarrier(command_buffer.getCommandBufer(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0u, nullptr, 1u, &barrier, 0u, nullptr);
}