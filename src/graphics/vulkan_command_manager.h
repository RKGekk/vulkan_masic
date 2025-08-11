#pragma once

#include <memory>
#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "vulkan_queue_family.h"

class VulkanCommandManager {
public:
    bool init(VkPhysicalDevice physical_device, VkDevice logical_device, VkSurfaceKHR surface);
    void destroy();

    VkCommandPool getGrapicsCommandPool() const;
    VkCommandPool getTransferCommandPool() const;
    VkCommandPool getComputeCommandPool() const;

    VkCommandBuffer allocCommandBuffer(VkCommandPool cmmand_pool, const VkCommandBufferAllocateInfo* command_buffer_info = nullptr) const;
    std::vector<VkCommandBuffer> allocCommandBuffer(VkCommandPool cmmand_pool, size_t buffers_count, VkCommandBufferAllocateInfo* command_buffer_info = nullptr) const;

	static void beginCommandBuffer(VkCommandBuffer command_buffer, VkCommandBufferBeginInfo* p_begin_info = nullptr);
	static void endCommandBuffer(VkCommandBuffer command_buffer);

    VkCommandBuffer beginSingleTimeCommands(VkCommandPool command_pool) const;
    void endSingleTimeCommands(VkCommandBuffer command_buffer, VkQueue queue, VkCommandPool command_pool) const;

    static void submitCommandBuffer(VkQueue queue, VkCommandBuffer command_buffer, VkFence fence = VK_NULL_HANDLE);
	static void submitCommandBuffer(VkQueue queue, const std::vector<VkCommandBuffer>& command_buffers, VkFence fence = VK_NULL_HANDLE);
    static void submitCommandBuffer(VkQueue queue, const VkSubmitInfo& submit_info, VkFence fence = VK_NULL_HANDLE);

    const QueueFamilyIndices& getQueueFamilyIndices() const;
    VkQueue getGraphicsQueue() const;
    VkQueue getComputeQueue() const;
    VkQueue getTransferQueue() const;
    VkQueue getPresentQueue() const;

    void transitionImageLayout(VkCommandPool cmd_pool, VkQueue queue, VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout, uint32_t mip_levels) const;
    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout, uint32_t mip_levels) const;

    void copyBufferToImage(VkCommandPool cmd_pool, VkQueue queue, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) const;
    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) const;
    void generateMipmaps(VkCommandPool cmd_pool, VkQueue queue, VkImage image, VkFormat image_format, int32_t tex_width, uint32_t tex_height, uint32_t mip_levels) const;
    void generateMipmaps(VkImage image, VkFormat image_format, int32_t tex_width, uint32_t tex_height, uint32_t mip_levels) const;
    void copyBuffer(VkCommandPool cmd_pool, VkQueue queue, VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size) const;
    void copyBuffer(VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size) const;

private:
    void createCommandPools();

    VkDevice m_device;
    VkCommandPool m_grapics_cmd_pool = VK_NULL_HANDLE;
    VkCommandPool m_transfer_cmd_pool = VK_NULL_HANDLE;
    VkCommandPool m_compute_cmd_pool = VK_NULL_HANDLE;

    QueueFamilyIndices m_queue_family_indices;
    VkQueue m_graphics_queue = VK_NULL_HANDLE;
    VkQueue m_compute_queue = VK_NULL_HANDLE;
    VkQueue m_transfer_queue = VK_NULL_HANDLE;
    VkQueue m_present_queue = VK_NULL_HANDLE;
};