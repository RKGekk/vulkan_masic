#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

#include "vulkan_command_pool_type.h"
#include "../pod/render_resource.h"

class VulkanSemaphoresManager;
class VulkanFenceManager;
class VulkanSemaphore;
class VulkanFence;

class CommandBatch {
public:
    struct CommandBatchInfo {
        PoolTypeEnum pool_type;
        VkCommandPool command_pool;
        VkCommandBufferLevel command_level;
        uint32_t family_index;
    };

    CommandBatch(VkDevice device, std::string name);

    bool init(std::vector<VkCommandBuffer> command_buffers, std::shared_ptr<CommandBatchInfo> batch_info);
    bool init(size_t count, std::shared_ptr<CommandBatchInfo> batch_info);
    void destroy();

    VkCommandBuffer getCommandBufer(size_t index = 0u) const;
    const VkCommandBuffer* getCommandBuferPtr(size_t index = 0u) const;
    size_t getCommandBuferCount() const;
    const std::vector<VkCommandBuffer>& getCommandBufers() const;
    const std::vector<std::shared_ptr<RenderResource>>& getResources() const;
    const std::shared_ptr<CommandBatchInfo>& getCommandBatchInfo() const;
    const VkCommandBufferAllocateInfo& getAllocateInfo() const;

    void addCommandBufer(VkCommandBuffer command_buffer);
    void addCommandBufer(std::vector<VkCommandBuffer> command_buffers);
    void addResource(std::shared_ptr<RenderResource> resource);

    PoolTypeEnum getPoolType() const;
    unsigned int getId() const;
    const uint32_t getFamilyIndex() const;
    const std::string& getName() const;

    void reset() const;

private:
    VkDevice m_device;

    std::string m_name;
    std::shared_ptr<CommandBatchInfo> m_batch_info;
    VkCommandBufferAllocateInfo m_allocate_info;

    std::vector<VkCommandBuffer> m_command_buffers;
    std::vector<std::shared_ptr<RenderResource>> m_resources;
};