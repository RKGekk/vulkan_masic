#include "vulkan_command_buffer.h"

#include <iterator>

bool CommandBatch::init(VkDevice device, std::vector<VkCommandBuffer> command_buffers, PoolTypeEnum pool_type, unsigned int id, BatchWaitInfo wait_info) {
    m_device = device;
    m_submit_id = id;
    m_command_buffers = command_buffers;
    m_pool_type = pool_type;
    m_wait_info = wait_info;

    VkSemaphoreCreateInfo buffer_available_sema_info{};
    buffer_available_sema_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VkResult result = vkCreateSemaphore(m_device, &buffer_available_sema_info, nullptr, &m_buffer_in_use_semaphore);
    if(result != VK_SUCCESS) {
        throw std::runtime_error("failed to create semaphore!");
    }

    VkFenceCreateInfo fen_info{};
    fen_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fen_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    result = vkCreateFence(m_device, &fen_info, nullptr, &m_render_fence);
    if(result != VK_SUCCESS) {
        throw std::runtime_error("failed to create fence!");
    }

    return true;
}

bool CommandBatch::init(VkDevice device, std::vector<VkCommandBuffer> command_buffers, VkSemaphore semaphore, VkFence fence, PoolTypeEnum pool_type, unsigned int submit_id, BatchWaitInfo wait_info) {
    m_device = device;
    m_submit_id = submit_id;
    m_command_buffers = command_buffers;
    m_pool_type = pool_type;
    m_buffer_in_use_semaphore = semaphore;
    m_render_fence = fence;

    return true;
}

bool CommandBatch::init(VkDevice device, size_t reserve, PoolTypeEnum pool_type, unsigned int id, BatchWaitInfo wait_info) {
    m_device = device;
    m_submit_id = id;
    m_command_buffers.reserve(reserve);
    m_pool_type = pool_type;
    m_wait_info = wait_info;

    VkSemaphoreCreateInfo buffer_available_sema_info{};
    buffer_available_sema_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VkResult result = vkCreateSemaphore(m_device, &buffer_available_sema_info, nullptr, &m_buffer_in_use_semaphore);
    if(result != VK_SUCCESS) {
        throw std::runtime_error("failed to create semaphore!");
    }

    VkFenceCreateInfo fen_info{};
    fen_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fen_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    result = vkCreateFence(m_device, &fen_info, nullptr, &m_render_fence);
    if(result != VK_SUCCESS) {
        throw std::runtime_error("failed to create fence!");
    }

    return true;
}

bool CommandBatch::init(VkDevice device, size_t reserve, VkSemaphore semaphore, VkFence fence, PoolTypeEnum pool_type, unsigned int submit_id, BatchWaitInfo wait_info) {
    m_device = device;
    m_submit_id = submit_id;
    m_command_buffers.reserve(reserve);
    m_pool_type = pool_type;
    m_buffer_in_use_semaphore = semaphore;
    m_render_fence = fence;

    return true;
}


void CommandBatch::destroy() {
    vkDestroySemaphore(m_device, m_buffer_in_use_semaphore, nullptr);
    vkDestroyFence(m_device, m_render_fence, nullptr);
}

VkCommandBuffer CommandBatch::getCommandBufer(size_t index) const {
    return m_command_buffers[index];
}

const VkCommandBuffer* CommandBatch::getCommandBuferPtr(size_t index) const {
    return &m_command_buffers[index];
}

size_t CommandBatch::getCommandBuferCount() const {
    return m_command_buffers.size();
}

const std::vector<VkCommandBuffer>& CommandBatch::getCommandBufers() const {
    return m_command_buffers;
}

const std::vector<std::shared_ptr<RenderResource>>& CommandBatch::getResources() const {
    return m_resources;
}

void CommandBatch::addCommandBufer(VkCommandBuffer command_buffer) {
    m_command_buffers.push_back(command_buffer);
}

void CommandBatch::addCommandBufer(std::vector<VkCommandBuffer> command_buffers) {
    m_command_buffers.insert(
        m_command_buffers.end(),
        std::make_move_iterator(command_buffers.begin()),
        std::make_move_iterator(command_buffers.end())
    );
}

void CommandBatch::reserveCommandBufer(size_t sz) {
    m_command_buffers.reserve(sz);
}

VkSemaphore CommandBatch::getInProgressSemaphore() const {
    return m_buffer_in_use_semaphore;
}

const VkSemaphore* CommandBatch::getInProgressSemaphorePtr() const {
    return &m_buffer_in_use_semaphore;
}

VkFence CommandBatch::getRenderFence() const {
    return m_render_fence;
}

const VkFence* CommandBatch::getRenderFencePtr() const {
    return &m_render_fence;
}

PoolTypeEnum CommandBatch::getPoolType() const {
    return m_pool_type;
}

unsigned int CommandBatch::getId() const {
    return m_submit_id;
}

VkSubmitInfo CommandBatch::getSubmitInfo(BatchWaitInfo* wait_info) const {
    VkSubmitInfo submit_info = {};
    if (wait_info) {
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.pNext = nullptr;
        submit_info.waitSemaphoreCount = static_cast<uint32_t>(wait_info->wait_for_semaphores.size());
        submit_info.pWaitSemaphores = wait_info->wait_for_semaphores.data();
        submit_info.pWaitDstStageMask = wait_info->wait_for_stages.data();
        submit_info.commandBufferCount = static_cast<uint32_t>(m_command_buffers.size());
        submit_info.pCommandBuffers = m_command_buffers.data();
        submit_info.signalSemaphoreCount = 1u;
        submit_info.pSignalSemaphores = &m_buffer_in_use_semaphore;
    }
    else {
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.pNext = nullptr;
        submit_info.waitSemaphoreCount = 0u;
        submit_info.pWaitSemaphores = nullptr;
        submit_info.pWaitDstStageMask = nullptr;
        submit_info.commandBufferCount = static_cast<uint32_t>(m_command_buffers.size());
        submit_info.pCommandBuffers = m_command_buffers.data();
        submit_info.signalSemaphoreCount = 1u;
        submit_info.pSignalSemaphores = &m_buffer_in_use_semaphore;
    }

    return submit_info;
}

const CommandBatch::BatchWaitInfo& CommandBatch::getWaitInfo() const {
    return m_wait_info;
}

void CommandBatch::setNoop() {
    m_noop = true;
}

bool CommandBatch::Noop() {
    return m_noop;
}

void CommandBatch::reset() const {
    for(VkCommandBuffer buffer : m_command_buffers) {
        vkResetCommandBuffer(buffer, 0u);
    }
    vkResetFences(m_device, 1u, &m_render_fence);
}