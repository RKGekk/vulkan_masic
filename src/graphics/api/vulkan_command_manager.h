#pragma once

#include <memory>
#include <string>
#include <unordered_map>
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
    struct CommandBufferPools {
        std::vector<VkCommandPool> primary_buffer_command_pools;
        std::vector<VkCommandPool> secondary_buffer_command_pools;
    };

    struct BatchWaitInfo {
        std::vector<VkSemaphore> wait_for_semaphores;
        std::vector<VkPipelineStageFlags> wait_for_stages;
        VkFence signal_fence;
        std::vector<VkSemaphore> signal_semaphores;
    };

    bool init(VkPhysicalDevice physical_device, VkDevice logical_device, VkSurfaceKHR surface, size_t frames_in_flight, size_t number_of_threads);
    void destroy();

    const QueueFamilyIndices& getQueueFamilyIndices() const;
    VkSharingMode getBufferSharingMode() const;
    const CommandBufferPools& getCommandPool(PoolTypeEnum pool_type = PoolTypeEnum::GRAPICS) const;
    void resetGfxPool(size_t frame_id) const;
    VkQueue getQueue(PoolTypeEnum pool_type = PoolTypeEnum::GRAPICS) const;

    std::shared_ptr<CommandBatch> allocCommandBufferPtr(PoolTypeEnum pool_type, VkCommandBufferLevel command_level, size_t frame_id, size_t thread_id, size_t buffers_count = 1u, std::string name = "no_name");

    void submitCommandBuffer(std::shared_ptr<CommandBatch>& command_buffer, std::shared_ptr<BatchWaitInfo> wait_info = nullptr, size_t index = 0u, VkSubmitInfo* p_submit_info = nullptr);
    void wait(PoolTypeEnum pool_type);

    void transitionImageLayout(VkCommandBuffer command_buffer, VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout, uint32_t mip_levels);
    void copyBufferToImage(VkCommandBuffer command_buffer, VkBuffer buffer, VkImage image, VkExtent3D extent, VkImageLayout image_layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    void generateMipmaps(VkCommandBuffer command_buffer, VkImage image, VkFormat format, VkExtent2D extent, uint32_t mip_levels, VkImageLayout initial_layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VkImageLayout post_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    void copyBuffer(VkCommandBuffer command_buffer, VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size);

private:
    void createCommandPools(size_t frames_in_flight, size_t number_of_threads);

    VkDevice m_device;
    std::unordered_map<PoolTypeEnum, CommandBufferPools> m_command_pools;
    int m_max_frames;
    int m_max_threads;

    QueueFamilyIndices m_queue_family_indices;
    VkQueue m_graphics_queue = VK_NULL_HANDLE;
    VkQueue m_compute_queue = VK_NULL_HANDLE;
    VkQueue m_transfer_queue = VK_NULL_HANDLE;
};