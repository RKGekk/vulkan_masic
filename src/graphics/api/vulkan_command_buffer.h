#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

#include "vulkan_command_pool_type.h"
#include "../pod/render_resource.h"

class CommandBatch {
public:
    struct BatchWaitInfo {
        std::vector<VkSemaphore> wait_for_semaphores;
        std::vector<VkPipelineStageFlags> wait_for_stages;
    };

    bool init(VkDevice device, std::vector<VkCommandBuffer> command_buffers, PoolTypeEnum pool_type, uint32_t family_index, unsigned int submit_id, BatchWaitInfo wait_info = {});
    bool init(VkDevice device, std::vector<VkCommandBuffer> command_buffers, VkSemaphore semaphore, VkFence fence, PoolTypeEnum pool_type, uint32_t family_index, unsigned int submit_id, BatchWaitInfo wait_info = {});
    bool init(VkDevice device, size_t reserve, PoolTypeEnum pool_type, uint32_t family_index, unsigned int submit_id, BatchWaitInfo wait_info = {});
    bool init(VkDevice device, size_t reserve, VkSemaphore semaphore, VkFence fence, PoolTypeEnum pool_type, uint32_t family_index, unsigned int submit_id, BatchWaitInfo wait_info = {});
    void destroy();

    VkCommandBuffer getCommandBufer(size_t index = 0u) const;
    const VkCommandBuffer* getCommandBuferPtr(size_t index = 0u) const;
    size_t getCommandBuferCount() const;
    const std::vector<VkCommandBuffer>& getCommandBufers() const;
    const std::vector<std::shared_ptr<RenderResource>>& getResources() const;
    void addCommandBufer(VkCommandBuffer command_buffer);
    void addCommandBufer(std::vector<VkCommandBuffer> command_buffers);
    void addResource(std::shared_ptr<RenderResource> resource);
    void reserveCommandBufer(size_t sz);

    VkSemaphore getInProgressSemaphore() const;
    const VkSemaphore* getInProgressSemaphorePtr() const;
    VkFence getRenderFence() const;
    const VkFence* getRenderFencePtr() const;
    PoolTypeEnum getPoolType() const;
    unsigned int getId() const;
    VkSubmitInfo getSubmitInfo(BatchWaitInfo* wait_info = nullptr) const;
    const BatchWaitInfo& getWaitInfo() const;
    const uint32_t getFamilyIndex() const;

    void setNoop();
    bool Noop();

    void reset() const;

private:
    VkDevice m_device;

    unsigned int m_submit_id;

    std::vector<VkCommandBuffer> m_command_buffers;
    std::vector<std::shared_ptr<RenderResource>> m_resources;
    VkSemaphore m_buffer_in_use_semaphore = VK_NULL_HANDLE;
    VkFence m_render_fence = VK_NULL_HANDLE;
    VkSubmitInfo m_submit_info;
    BatchWaitInfo m_wait_info;
    uint32_t m_family_index;

    bool m_noop = false;

    PoolTypeEnum m_pool_type;
};