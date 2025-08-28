#include "vulkan_command_buffer.h"

bool CommandBuffer::init(VkDevice device, VkCommandBuffer command_buffer, PoolTypeEnum pool_type, unsigned int id) {
    m_device = device;
    m_id = id;
    m_command_buffer = command_buffer;
    m_pool_type = pool_type;

    VkSemaphoreCreateInfo buffer_available_sema_info{};
    buffer_available_sema_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VkResult result = vkCreateSemaphore(m_device, &buffer_available_sema_info, nullptr, &m_buffer_in_use_semaphore);
    if(result != VK_SUCCESS) {
        throw std::runtime_error("failed to create semaphore!");
    }
}

void CommandBuffer::destroy() {
    vkDestroySemaphore(m_device, m_buffer_in_use_semaphore, nullptr);
}

VkCommandBuffer CommandBuffer::getCommandBufer() const {
    return m_command_buffer;
}
    
VkSemaphore CommandBuffer::getInProgressSemaphore() const {
    return m_buffer_in_use_semaphore;
}
    
PoolTypeEnum CommandBuffer::getPoolType() const {
    return m_pool_type;
}

unsigned int CommandBuffer::getId() const {
    return m_id;
}

void CommandBuffer::reset() {
    vkResetCommandBuffer(m_command_buffer, 0u);
}