#pragma once

#include <memory>
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
    static constexpr size_t SELECT_ALL_BUFFERS = -1;
    static constexpr uint32_t MAX_COMMAND_BUFFERS = 16u;

    bool init(VkPhysicalDevice physical_device, VkDevice logical_device, VkSurfaceKHR surface, std::shared_ptr<ThreadPool> thread_pool);
    void destroy();

    const QueueFamilyIndices& getQueueFamilyIndices() const;
    VkSharingMode getBufferSharingMode() const;
    VkCommandPool getCommandPool(PoolTypeEnum pool_type = PoolTypeEnum::GRAPICS) const;
    VkQueue getQueue(PoolTypeEnum pool_type = PoolTypeEnum::GRAPICS) const;

    CommandBatch allocCommandBuffer(PoolTypeEnum pool_type = PoolTypeEnum::GRAPICS, size_t buffers_count = 1u, CommandBatch::BatchWaitInfo wait_info = {}, VkCommandBufferAllocateInfo* command_buffer_info = nullptr);

	static void beginCommandBuffer(CommandBatch& command_buffer, size_t index = 0u, VkCommandBufferBeginInfo* p_begin_info = nullptr);
	static void endCommandBuffer(CommandBatch& command_buffer, size_t index = 0u);

    void submitCommandBuffer(CommandBatch& command_buffer, size_t index = 0u, VkSubmitInfo* p_submit_info = nullptr);
    void wait(PoolTypeEnum pool_type);

    void transitionImageLayout(VkCommandBuffer command_buffer, VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout, uint32_t mip_levels);
    void copyBufferToImage(VkCommandBuffer command_buffer, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
    void generateMipmaps(VkCommandBuffer command_buffer, VkImage image, VkFormat image_format, int32_t tex_width, uint32_t tex_height, uint32_t mip_levels);
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

    ThreadSafeQueue<CommandBatch> m_work_in_progress;

    std::vector<VkCommandBuffer> m_command_buffers;
    ThreadSafeLookupTable<unsigned int, std::vector<size_t>> m_submit_to_buf_id;
    ThreadSafeQueue<size_t> m_free_cmd_idx;

    std::vector<VkSemaphore> m_semaphores;
    ThreadSafeLookupTable<unsigned int, size_t> m_submit_to_sem_id;
    ThreadSafeQueue<size_t> m_free_smp_idx;

    std::vector<VkFence> m_fences;
    ThreadSafeLookupTable<unsigned int, size_t> m_submit_to_fnc_id;
    ThreadSafeQueue<size_t> m_free_fnc_idx;

    std::shared_ptr<ThreadPool> m_thread_pool;
    scope_thread m_cmd_buff_thread;
};