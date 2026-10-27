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

    bool init(std::shared_ptr<VulkanFramebuffer> fb, const std::shared_ptr<VulkanCommandManager>& cmd_mgr, size_t command_buffer_count, std::string name);
    void destroy();

    const std::shared_ptr<CommandBatch>& getPrimaryCommandBuffer() const;
    const std::shared_ptr<CommandBatch>& getSecondaryCommandBuffer() const;

    const std::shared_ptr<VulkanFramebuffer>& getFrameBuffer() const;

    void beginCommandBuffer(const std::string& subpass_name, const ResourcesStateTransitionFn& transition_fn) const;
    void endCommandBuffer() const;

private:
    std::string m_name;
    std::shared_ptr<CommandBatch> m_primary_command_buffer;
    std::shared_ptr<CommandBatch> m_secondary_command_buffer;
    std::shared_ptr<VulkanFramebuffer> m_frame_buffer;
};