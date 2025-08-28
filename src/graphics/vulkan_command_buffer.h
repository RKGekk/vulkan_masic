#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdexcept>

#include "vulkan_command_pool_type.h"

class CommandBuffer {
public:
    bool init(VkDevice m_device, VkCommandBuffer command_buffer, PoolTypeEnum pool_type, unsigned int id);
    void destroy();

    VkCommandBuffer getCommandBufer() const;
    VkSemaphore getInProgressSemaphore() const;
    PoolTypeEnum getPoolType() const;
    unsigned int getId() const;

    void reset();

private:
    VkDevice m_device;

    unsigned int m_id;

    VkCommandBuffer m_command_buffer = VK_NULL_HANDLE;
    VkSemaphore m_buffer_in_use_semaphore = VK_NULL_HANDLE;
    VkSubmitInfo submit_info;

    PoolTypeEnum m_pool_type;
};