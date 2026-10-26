#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "vulkan_queue_family.h"
#include "vulkan_command_pool_type.h"
#include "vulkan_command_buffer.h"
#include "../../tools/thread_safe_queue.h"
#include "../../tools/thread_pool.h"
#include "../../tools/thread_safe_lookup_table.h"

class VulkanCommandManager {
public:
    bool init(VkPhysicalDevice physical_device, VkDevice logical_device, VkSurfaceKHR surface);
    void destroy();

    const QueueFamilyIndices& getQueueFamilyIndices() const;
    VkSharingMode getBufferSharingMode() const;
    VkCommandPool getCommandPool(PoolTypeEnum pool_type = PoolTypeEnum::GRAPICS) const;
    VkQueue getQueue(PoolTypeEnum pool_type = PoolTypeEnum::GRAPICS) const;

    std::shared_ptr<CommandBatch> allocCommandBufferPtr(PoolTypeEnum pool_type, VkCommandBufferLevel command_level, size_t buffers_count = 1u, std::string name = "no_name");

	static void beginCommandBuffer(std::shared_ptr<CommandBatch>& command_buffer, size_t index = 0u, VkCommandBufferBeginInfo* p_begin_info = nullptr);
	static void endCommandBuffer(std::shared_ptr<CommandBatch>& command_buffer, size_t index = 0u);

    void submitCommandBuffer(std::shared_ptr<CommandBatch>& command_buffer, size_t index = 0u, VkSubmitInfo* p_submit_info = nullptr);
    void wait(PoolTypeEnum pool_type);

    void transitionImageLayout(VkCommandBuffer command_buffer, VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout, uint32_t mip_levels);
    void copyBufferToImage(VkCommandBuffer command_buffer, VkBuffer buffer, VkImage image, VkExtent3D extent, VkImageLayout image_layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    void generateMipmaps(VkCommandBuffer command_buffer, VkImage image, VkFormat format, VkExtent2D extent, uint32_t mip_levels, VkImageLayout initial_layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VkImageLayout post_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    void copyBuffer(VkCommandBuffer command_buffer, VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size);

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