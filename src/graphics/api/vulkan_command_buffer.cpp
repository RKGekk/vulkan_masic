#include "vulkan_command_buffer.h"

#include <iterator>

#include "../../application.h"
#include "vulkan_fence_manager.h"
#include "vulkan_semaphores_manager.h"
#include "vulkan_fence.h"
#include "vulkan_semaphore.h"

CommandBatch::CommandBatch(VkDevice device, std::string name) : m_device(device), m_name(name) {}

bool CommandBatch::init(std::vector<VkCommandBuffer> command_buffers, std::shared_ptr<CommandBatchInfo> batch_info) {
    m_batch_info = std::move(batch_info);
    m_command_buffers = std::move(command_buffers);

    return true;
}

bool CommandBatch::init(size_t count, std::shared_ptr<CommandBatchInfo> batch_info) {
    using namespace std::literals;

    m_batch_info = std::move(batch_info);
    m_command_buffers.resize(count);

    m_allocate_info = {};
    m_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    m_allocate_info.commandPool = m_batch_info->command_pool;
    m_allocate_info.level = m_batch_info->command_level;
    m_allocate_info.commandBufferCount = static_cast<uint32_t>(count);

	VkResult result = vkAllocateCommandBuffers(m_device, &m_allocate_info, m_command_buffers.data());
    if(result != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }

#ifndef NDEBUG
    for (size_t i = 0u; i < count; ++i) {
        std::string cmd_buff_name = "command_buffer_"s + m_name + "_"s + std::to_string(i);
        auto vkSetDebugUtilsObjectNameEXT = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetInstanceProcAddr(Application::GetInstance().getInstance(), "vkSetDebugUtilsObjectNameEXT");
        VkDebugUtilsObjectNameInfoEXT name_info = {};
        name_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        name_info.objectType = VK_OBJECT_TYPE_COMMAND_BUFFER;
        name_info.objectHandle = (uint64_t)m_command_buffers[i];
        name_info.pObjectName = cmd_buff_name.c_str();

        vkSetDebugUtilsObjectNameEXT(m_device, &name_info);
    }
#endif

    return true;
}

void CommandBatch::destroy() {}

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

const std::shared_ptr<CommandBatch::CommandBatchInfo>& CommandBatch::getCommandBatchInfo() const {
    return m_batch_info;
}

const VkCommandBufferAllocateInfo& CommandBatch::getAllocateInfo() const {
    return m_allocate_info;
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

PoolTypeEnum CommandBatch::getPoolType() const {
    return m_batch_info->pool_type;
}

const uint32_t CommandBatch::getFamilyIndex() const {
    return m_batch_info->family_index;
}

const std::string& CommandBatch::getName() const {
    return m_name;
}

void CommandBatch::reset() const {
    for(VkCommandBuffer buffer : m_command_buffers) {
        vkResetCommandBuffer(buffer, 0u);
        //vkResetCommandBuffer(buffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
    }
}