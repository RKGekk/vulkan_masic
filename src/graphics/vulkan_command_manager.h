#pragma once

#include <memory>
#include <utility>
#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "vulkan_queue_family.h"
#include "vulkan_command_pool_type.h"
#include "vulkan_command_buffer.h"

class VulkanCommandManager {
public:
    bool init(VkPhysicalDevice physical_device, VkDevice logical_device, VkSurfaceKHR surface);
    void destroy();

    const QueueFamilyIndices& getQueueFamilyIndices() const;
    VkSharingMode getBufferSharingMode() const;
    VkCommandPool getCommandPool(PoolTypeEnum pool_type = PoolTypeEnum::GRAPICS) const;
    VkQueue getQueue(PoolTypeEnum pool_type = PoolTypeEnum::GRAPICS) const;

    CommandBuffer allocCommandBuffer(PoolTypeEnum pool_type = PoolTypeEnum::GRAPICS, const VkCommandBufferAllocateInfo* command_buffer_info = nullptr) const;
    std::vector<CommandBuffer> allocCommandBuffer(size_t buffers_count, PoolTypeEnum pool_type = PoolTypeEnum::GRAPICS, VkCommandBufferAllocateInfo* command_buffer_info = nullptr) const;

	static void beginCommandBuffer(CommandBuffer command_buffer, VkCommandBufferBeginInfo* p_begin_info = nullptr);
	static void endCommandBuffer(CommandBuffer command_buffer);

    CommandBuffer beginSingleTimeCommands(PoolTypeEnum pool_type = PoolTypeEnum::GRAPICS) const;
    void endSingleTimeCommands(CommandBuffer command_buffer) const;

    void submitCommandBuffer(CommandBuffer command_buffer, PoolTypeEnum pool_type = PoolTypeEnum::GRAPICS, VkFence fence = VK_NULL_HANDLE) const;
	void submitCommandBuffer(const std::vector<CommandBuffer>& command_buffers, PoolTypeEnum pool_type = PoolTypeEnum::GRAPICS, VkFence fence = VK_NULL_HANDLE) const;
    void submitCommandBuffer(const VkSubmitInfo& submit_info, PoolTypeEnum pool_type = PoolTypeEnum::GRAPICS, VkFence fence = VK_NULL_HANDLE) const;

    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout, uint32_t mip_levels, PoolTypeEnum pool_type = PoolTypeEnum::GRAPICS) const;
    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, PoolTypeEnum pool_type = PoolTypeEnum::GRAPICS) const;
    void generateMipmaps(VkImage image, VkFormat image_format, int32_t tex_width, uint32_t tex_height, uint32_t mip_levels, PoolTypeEnum pool_type = PoolTypeEnum::GRAPICS) const;
    void copyBuffer(VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size, PoolTypeEnum pool_type = PoolTypeEnum::GRAPICS) const;

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
};