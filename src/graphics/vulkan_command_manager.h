#pragma once

#include <memory>
#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "vulkan_device.h"

class VulkanCommandManager {
public:
    bool init(std::shared_ptr<VulkanDevice> device);
    void destroy();

    VkCommandPool getGrapicsCommandPool() const;
    VkCommandPool getTransferCommandPool() const;
    VkCommandPool getComputeCommandPool() const;

    static VkCommandBuffer allocCommandBuffer(VkDevice device, VkCommandPool cmmand_pool, const VkCommandBufferAllocateInfo* command_buffer_info = nullptr);
    static void allocCommandBuffer(VkDevice device, VkCommandPool cmmand_pool, std::vector<VkCommandBuffer>& command_buffers, VkCommandBufferAllocateInfo* command_buffer_info = nullptr);
	static void beginCommandBuffer(VkCommandBuffer command_buffer, VkCommandBufferBeginInfo* p_begin_info = nullptr);
	static void endCommandBuffer(VkCommandBuffer command_buffer);
    static VkCommandBuffer beginSingleTimeCommands(VkDevice device, VkCommandPool command_pool);
    static void endSingleTimeCommands(VkDevice device, VkCommandBuffer command_buffer, VkQueue queue, VkCommandPool command_pool);
    static void submitCommandBuffer(VkQueue queue, VkCommandBuffer command_buffer, VkFence fence = VK_NULL_HANDLE);
    static void submitCommandBuffer(VkQueue queue, const VkSubmitInfo& submit_info, VkFence fence = VK_NULL_HANDLE);
	static void submitCommandBuffer(VkQueue queue, const std::vector<VkCommandBuffer>& command_buffers, VkFence fence = VK_NULL_HANDLE);


private:
    void createCommandPools();

    std::shared_ptr<VulkanDevice> m_device;
    VkCommandPool m_grapics_cmd_pool = VK_NULL_HANDLE;
    VkCommandPool m_transfer_cmd_pool = VK_NULL_HANDLE;
    VkCommandPool m_compute_cmd_pool = VK_NULL_HANDLE;
};