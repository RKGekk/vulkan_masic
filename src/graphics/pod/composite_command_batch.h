#pragma once

#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "../api/vulkan_command_pool_type.h"

class VulkanCommandManager;
class VulkanFramebuffer;
class CommandBatch;

class CompositeCommandBatch {
public:
    using ResourcesStateTransitionFn = std::function<void(VkCommandBuffer)>;

    bool init(std::shared_ptr<VulkanFramebuffer> fb, size_t frame_id, size_t thread_id, size_t command_buffer_count, std::string name);
    void destroy();

    const std::shared_ptr<CommandBatch>& getPrimaryCommandBuffer() const;
    const std::shared_ptr<CommandBatch>& getSecondaryCommandBuffer() const;

    const std::shared_ptr<VulkanFramebuffer>& getFrameBuffer() const;

    void beginPrimaryCommandBuffer(const ResourcesStateTransitionFn& transition_fn);
    void beginSecondaryCommandBuffer(const std::string& subpass_name, size_t index = CommandBatch::SELECT_ALL_BUFFERS);
    void endPrimaryCommandBuffer();
    void endSecondaryCommandBuffer(size_t index = CommandBatch::SELECT_ALL_BUFFERS);
    void resetCommandBuffer(VkCommandBufferLevel lvl, size_t index = CommandBatch::SELECT_ALL_BUFFERS);

    bool isRecorded(VkCommandBufferLevel lvl, size_t index = CommandBatch::SELECT_ALL_BUFFERS) const;

private:
    std::string m_name;
    bool m_recorded_primary;
    std::vector<bool> m_recorded_secondary;
    std::shared_ptr<CommandBatch> m_primary_command_buffer;
    std::shared_ptr<CommandBatch> m_secondary_command_buffer;
    std::shared_ptr<VulkanFramebuffer> m_frame_buffer;
    std::shared_ptr<VulkanCommandManager> m_command_manager;
};