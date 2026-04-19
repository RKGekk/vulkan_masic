#include "vulkan_buffer.h"

#include "vulkan_device.h"
#include "vulkan_command_buffer.h"
#include "../pod/buffer_config.h"
#include "../pod/format_config.h"
#include "../../application.h"
#include "../vulkan_renderer.h"
#include "vulkan_resources_manager.h"

VulkanBuffer::VulkanBuffer(std::shared_ptr<VulkanDevice> device, std::string name) : m_device(std::move(device)), m_name(std::move(name)) {}
VulkanBuffer::VulkanBuffer(std::shared_ptr<VulkanDevice> device) : m_device(std::move(device)), m_name(std::to_string(rand())) {};

bool VulkanBuffer::init(const void* data, std::shared_ptr<BufferConfig> buffer_config) {
    using namespace std::literals;

    m_buffer_config = buffer_config;

    if (m_buffer_config->isSizeDynamic()) return true;

    VkResult result = vkCreateBuffer(m_device->getDevice(), &m_buffer_config->getBufferInfo(), nullptr, &m_buffer);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to create buffer!");
    }

#ifndef NDEBUG
    std::string buffer_name = "buffer_"s + m_name;
    auto vkSetDebugUtilsObjectNameEXT = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetInstanceProcAddr(Application::GetInstance().getInstance(), "vkSetDebugUtilsObjectNameEXT");
    VkDebugUtilsObjectNameInfoEXT name_info = {};
    name_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    name_info.objectType = VK_OBJECT_TYPE_BUFFER;
    name_info.objectHandle = (uint64_t)m_buffer;
    name_info.pObjectName = buffer_name.c_str();

    vkSetDebugUtilsObjectNameEXT(m_device->getDevice(), &name_info);
#endif    
    
    VkMemoryRequirements mem_req;
    vkGetBufferMemoryRequirements(m_device->getDevice(), m_buffer, &mem_req);
    uint32_t mem_type_idx = m_device->findMemoryType(mem_req.memoryTypeBits, m_buffer_config->getMemoryProperties());
    m_buffer_config->setAlignedSize(mem_req.size);
    m_buffer_config->setAlignment(mem_req.alignment);
  
    VkMemoryAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.pNext = nullptr;
    alloc_info.allocationSize = mem_req.size;
    alloc_info.memoryTypeIndex = mem_type_idx;
 
    result = vkAllocateMemory(m_device->getDevice(), &alloc_info, nullptr, &m_memory);
    if(result != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate buffer memory!");
    }
    VkDeviceSize offset = 0u;
    vkBindBufferMemory(m_device->getDevice(), m_buffer, m_memory, offset);

    if(m_buffer_config->getMemoryProperties() & (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
        vkMapMemory(m_device->getDevice(), m_memory, 0, mem_req.size, 0u, &m_mapped);
    }

    for(const auto&[view_type_name, view_cfg_ptr] : m_buffer_config->getViewMap()) {
        view_cfg_ptr->view_info.buffer = m_buffer;
        VkResult result = vkCreateBufferView(m_device->getDevice(), &view_cfg_ptr->view_info, nullptr, &m_buffer_view_map[view_type_name]);
        if(result != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture image view!");
        }

#ifndef NDEBUG
        std::string buffer_view_name = "buffer_view_"s + view_type_name;
        auto vkSetDebugUtilsObjectNameEXT = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetInstanceProcAddr(Application::GetInstance().getInstance(), "vkSetDebugUtilsObjectNameEXT");
        VkDebugUtilsObjectNameInfoEXT name_info = {};
        name_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        name_info.objectType = VK_OBJECT_TYPE_BUFFER_VIEW;
        name_info.objectHandle = (uint64_t)m_buffer_view_map[view_type_name];
        name_info.pObjectName = buffer_view_name.c_str();

        vkSetDebugUtilsObjectNameEXT(m_device->getDevice(), &name_info);
#endif    

    }
    
    if(data) {
        update(data, m_buffer_config->getNotAlignedSize());
    }
 
    return true;
}

bool VulkanBuffer::init(CommandBatch& command_buffer, const void* data, std::shared_ptr<BufferConfig> buffer_config) {
    VkDeviceSize data_size = m_buffer_config->getNotAlignedSize();
    init(nullptr, std::move(buffer_config));
    update(command_buffer, data, data_size);

    return true;
}

void VulkanBuffer::destroy() {
    if(m_buffer_config->getBufferInfo().size) {
        for (auto&[view_name, vk_view] : m_buffer_view_map) {
            vkDestroyBufferView(m_device->getDevice(), vk_view, nullptr);
            vk_view = VK_NULL_HANDLE;
        }

        if(m_buffer_config->getMemoryProperties() & (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
            vkUnmapMemory(m_device->getDevice(), m_memory);
        }
        vkDestroyBuffer(m_device->getDevice(), m_buffer, nullptr);
        vkFreeMemory(m_device->getDevice(), m_memory, nullptr);
        m_memory = VK_NULL_HANDLE;
        m_buffer = VK_NULL_HANDLE;
        m_mapped = nullptr;
        //m_buffer_config->destroy();
    }
}

VkBuffer VulkanBuffer::getBuffer() const {
    return m_buffer;
}

VkBuffer* VulkanBuffer::getBufferPtr() {
    return &m_buffer;
}

VkDeviceMemory VulkanBuffer::getMemory() const {
    return m_memory;
}

void* VulkanBuffer::getMappedBuffer() const {
    if (m_buffer_config->getMemoryProperties() & (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
        return m_mapped;
    }
    throw std::runtime_error("failed map device buffer to host!");
}

VkDeviceSize VulkanBuffer::getAlignedSize() const {
    return m_buffer_config->getBufferInfo().size;
}

VkDeviceSize VulkanBuffer::getNotAlignedSize() const {
    return m_buffer_config->getNotAlignedSize();
}

VkMemoryPropertyFlags VulkanBuffer::getProperties() const {
    return m_buffer_config->getMemoryProperties();
}

VkBufferUsageFlags VulkanBuffer::getUsage() const {
    return m_buffer_config->getBufferInfo().usage;
}

VkBufferView VulkanBuffer::getBufferView() const {
    return (*m_buffer_view_map.begin()).second;
};

VkBufferView VulkanBuffer::getBufferView(const std::string& view_name) const {
    return m_buffer_view_map.at(view_name);
}

const std::unordered_map<std::string, VkBufferView>& VulkanBuffer::getBufferViewMap() const {
    return m_buffer_view_map;
}

void VulkanBuffer::update(const void* src_data, VkDeviceSize buffer_size) {
    if (buffer_size > m_buffer_config->getBufferInfo().size && m_buffer_config->isSizeDynamic()) {
        destroy();
        m_buffer_config->setNotAlignedSize(buffer_size);
        m_buffer_config->setAlignedSize(buffer_size);
        m_buffer_config->setSizeDynamic(false);
        init(src_data, m_buffer_config);
        m_buffer_config->setSizeDynamic(true);
        return;
    }
    if(!src_data) {
        return;
    }
    if(m_buffer_config->getMemoryProperties() & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) {
        memcpy(m_mapped, src_data, buffer_size);
        return;
    }
    else if(m_buffer_config->getMemoryProperties() & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
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
        std::shared_ptr<CommandBatch> command_buffer_ptr = m_device->getCommandManager()->allocCommandBufferPtr(PoolTypeEnum::TRANSFER);
        update(*command_buffer_ptr, src_data, buffer_size);
        m_device->getCommandManager()->submitCommandBuffer(command_buffer_ptr);
        m_device->getCommandManager()->wait(PoolTypeEnum::TRANSFER);
        command_buffer_ptr->destroy();

        return;
    }
}

void VulkanBuffer::update(CommandBatch& command_buffer, const void* src_data, VkDeviceSize buffer_size, VkAccessFlags dstAccessMask) {
    if (buffer_size > m_buffer_config->getBufferInfo().size && m_buffer_config->isSizeDynamic()) {
        destroy();
        m_buffer_config->setNotAlignedSize(buffer_size);
        m_buffer_config->setAlignedSize(buffer_size);
        m_buffer_config->setSizeDynamic(false);
        init(src_data, m_buffer_config);
        m_buffer_config->setSizeDynamic(true);
        return;
    }
    if(!src_data) {
        return;
    }
    if(m_buffer_config->getMemoryProperties() & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) {
        memcpy(m_mapped, src_data, buffer_size);
        return;
    }
    if(m_buffer_config->getMemoryProperties() & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
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

    Application& app = Application::Get();
    VulkanRenderer& renderer = app.GetRenderer();
    std::shared_ptr<VulkanBuffer> staging_buffer = Application::Get().GetRenderer().getResourcesManager()->create_buffer(src_data, m_buffer_config->getBufferInfo().size, "basic_staging_buffer"s);
    command_buffer.addResource(staging_buffer);

    m_device->getCommandManager()->copyBuffer(command_buffer.getCommandBufer(), staging_buffer->getBuffer(), m_buffer, buffer_size);

    // alternative to vkCmdCopyBuffer from m_device->getCommandManager().copyBuffer, the data is consumed from host memory as soon as vkCmdUpdateBuffer() is called, Vulkan make a copy of the data you’ve supplied! Data is not written into the buffer until vkCmdUpdateBuffer() is executed by the device after the command buffer has been submitted! The maximum size of data that can be placed in a buffer with vkCmdUpdateBuffer() is 65,536 bytes.
    // vkCmdUpdateBuffer(command_buffer.getCommandBufer(), m_buffer, 0u, buffer_size, src_data);

    //setGlobalMemoryUpdateBarier(command_buffer, dstAccessMask);
    setMemoryUpdateBarier(command_buffer, dstAccessMask);
}

void VulkanBuffer::update(CommandBatch& command_buffer, const void* src_data, VkDeviceSize buffer_size) {
    update(command_buffer, src_data, buffer_size, VulkanDevice::getDstAccessMask(m_buffer_config->getBufferInfo().usage));
}

const RenderResource::ResourceName& VulkanBuffer::getName() const {
    return m_name;
}

RenderResource::Type VulkanBuffer::getType() const {
    return RenderResource::Type::BUFFER;
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