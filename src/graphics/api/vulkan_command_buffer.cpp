#include "vulkan_command_buffer.h"

#include <iterator>

#include "vulkan_fence_manager.h"
#include "vulkan_semaphores_manager.h"

CommandBatch::CommandBatch(VkDevice device, std::shared_ptr<VulkanSemaphoresManager> semaphores_manager, std::shared_ptr<VulkanFenceManager> fence_manager) : m_device(device), m_semaphores_manager(std::move(semaphores_manager)), m_fence_manager(std::move(fence_manager)) {
    m_buffer_in_use_semaphore = m_semaphores_manager->getSemaphore();
    m_render_fence = m_fence_manager->getFence();
}

bool CommandBatch::init(std::vector<VkCommandBuffer> command_buffers, PoolTypeEnum pool_type, uint32_t family_index, unsigned int id, BatchWaitInfo wait_info) {
    m_submit_id = id;
    m_command_buffers = command_buffers;
    m_pool_type = pool_type;
    m_wait_info = std::move(wait_info);
    m_family_index = family_index;

    return true;
}

bool CommandBatch::init(size_t reserve, PoolTypeEnum pool_type, uint32_t family_index, unsigned int id, BatchWaitInfo wait_info) {
    m_submit_id = id;
    m_command_buffers.reserve(reserve);
    m_pool_type = pool_type;
    m_wait_info = std::move(wait_info);
    m_family_index = family_index;

    return true;
}

void CommandBatch::destroy() {
    m_fence_manager->returnFence(m_render_fence);
    m_semaphores_manager->returnSemaphore(m_buffer_in_use_semaphore);
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

void CommandBatch::addResource(std::shared_ptr<RenderResource> resource) {
    m_resources.push_back(std::move(resource));
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
        submit_info.signalSemaphoreCount = 0u;
        submit_info.pSignalSemaphores = nullptr;
    }

    return submit_info;
}

const CommandBatch::BatchWaitInfo& CommandBatch::getWaitInfo() const {
    return m_wait_info;
}

const uint32_t CommandBatch::getFamilyIndex() const {
    return m_family_index;
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
        //vkResetCommandBuffer(buffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
    }
    vkResetFences(m_device, 1u, &m_render_fence);
}