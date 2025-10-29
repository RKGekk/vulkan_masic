#include "vulkan_command_manager.h"
#include "vulkan_device.h"
#include "render_resource.h"

unsigned int LAST_COMMAND_BUFFER_ID = 0u;

bool VulkanCommandManager::init(VkPhysicalDevice physical_device, VkDevice logical_device, VkSurfaceKHR surface, std::shared_ptr<ThreadPool> thread_pool) {
    m_device = logical_device;
    m_queue_family_indices.init(physical_device, surface);
    m_thread_pool = std::move(thread_pool);

    vkGetDeviceQueue(m_device, m_queue_family_indices.getFamilyIdx(PoolTypeEnum::GRAPICS).value(), 0u, &m_graphics_queue);
    vkGetDeviceQueue(m_device, m_queue_family_indices.getFamilyIdx(PoolTypeEnum::COMPUTE).value(), 0u, &m_compute_queue);
    vkGetDeviceQueue(m_device, m_queue_family_indices.getFamilyIdx(PoolTypeEnum::TRANSFER).value(), 0u, &m_transfer_queue);

    createCommandPools();

    m_command_buffers.resize(MAX_COMMAND_BUFFERS);
    VkCommandBufferAllocateInfo command_alloc_info{};
    command_alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_alloc_info.commandPool = getCommandPool(PoolTypeEnum::TRANSFER);
    command_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    command_alloc_info.commandBufferCount = MAX_COMMAND_BUFFERS;

	VkResult result = vkAllocateCommandBuffers(m_device, &command_alloc_info, m_command_buffers.data());
    if(result != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }

    m_fences.resize(MAX_COMMAND_BUFFERS);
    m_semaphores.resize(MAX_COMMAND_BUFFERS);
    for (uint32_t i = 0u; i < MAX_COMMAND_BUFFERS; ++i) {
        m_free_cmd_idx.Push(i);

        VkFenceCreateInfo fen_info{};
        fen_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        //fen_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        result = vkCreateFence(m_device, &fen_info, nullptr, &m_fences[i]);
        vkResetFences(m_device, 1u, &m_fences[i]);
        if(result != VK_SUCCESS) {
            throw std::runtime_error("failed to create fence!");
        }
        m_free_smp_idx.Push(i);
    
        VkSemaphoreCreateInfo buffer_available_sema_info{};
        buffer_available_sema_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        result = vkCreateSemaphore(m_device, &buffer_available_sema_info, nullptr, &m_semaphores[i]);
        if(result != VK_SUCCESS) {
            throw std::runtime_error("failed to create semaphore!");
        }
        m_free_fnc_idx.Push(i);
    }

    m_cmd_buff_thread = std::thread(
        [this]() {
            while(true) {
                CommandBatch work_in_progress;
                m_work_in_progress.WaitAndPop(work_in_progress);
                if(work_in_progress.Noop()) break;

                vkWaitForFences(m_device, 1u, work_in_progress.getRenderFencePtr(), VK_TRUE, UINT64_MAX);
                work_in_progress.reset();

                std::vector<size_t> cmd_idx = m_submit_to_buf_id.ValueFor(work_in_progress.getId(), {});
                for (size_t i : cmd_idx) {
                    m_free_cmd_idx.Push(i);
                }
                m_submit_to_buf_id.RemoveMapping(work_in_progress.getId());

                size_t sem_id = m_submit_to_sem_id.ValueFor(work_in_progress.getId(), -1);
                m_free_smp_idx.Push(sem_id);
                m_submit_to_sem_id.RemoveMapping(work_in_progress.getId());

                size_t fnc_id = m_submit_to_fnc_id.ValueFor(work_in_progress.getId(), -1);
                m_free_fnc_idx.Push(fnc_id);
                m_submit_to_fnc_id.RemoveMapping(work_in_progress.getId());

                for(const std::shared_ptr<RenderResource>& resource : work_in_progress.getResources()) {
                    resource->destroy();
                }
            }
            return;
        }
    );

    return true;
}

void VulkanCommandManager::destroy() {
    CommandBatch noop;
    noop.setNoop();
    m_work_in_progress.Push(std::move(noop));
    if(m_grapics_cmd_pool != m_transfer_cmd_pool) {
        vkDestroyCommandPool(m_device, m_grapics_cmd_pool, nullptr);
        vkDestroyCommandPool(m_device, m_transfer_cmd_pool, nullptr);
    }
    else {
        vkDestroyCommandPool(m_device, m_grapics_cmd_pool, nullptr);
    }

    for(size_t i = 0u; i < MAX_COMMAND_BUFFERS; ++i) {
        vkDestroySemaphore(m_device, m_semaphores[i], nullptr);
        vkDestroyFence(m_device, m_fences[i], nullptr);
    }
}

const QueueFamilyIndices& VulkanCommandManager::getQueueFamilyIndices() const {
    return m_queue_family_indices;
}

VkSharingMode VulkanCommandManager::getBufferSharingMode() const {
    return m_queue_family_indices.getBufferSharingMode();
}

VkQueue VulkanCommandManager::getQueue(PoolTypeEnum pool_type) const {
    switch (pool_type) {
        case PoolTypeEnum::GRAPICS : return m_graphics_queue;
        case PoolTypeEnum::TRANSFER : return m_transfer_queue;
        case PoolTypeEnum::COMPUTE : return m_compute_queue;
        default : return m_graphics_queue;
    }
    return m_graphics_queue;
}

VkCommandPool VulkanCommandManager::getCommandPool(PoolTypeEnum pool_type) const {
    switch (pool_type) {
        case PoolTypeEnum::GRAPICS : return m_grapics_cmd_pool;
        case PoolTypeEnum::TRANSFER : return m_transfer_cmd_pool;
        case PoolTypeEnum::COMPUTE : return m_compute_cmd_pool;
        default : return m_grapics_cmd_pool;
    }
    return m_grapics_cmd_pool;
}

CommandBatch VulkanCommandManager::allocCommandBuffer(PoolTypeEnum pool_type, size_t buffers_count, CommandBatch::BatchWaitInfo wait_info, VkCommandBufferAllocateInfo* command_buffer_info) {
    std::vector<VkCommandBuffer> command_buffers;
    command_buffers.resize(buffers_count);

    if(pool_type == PoolTypeEnum::TRANSFER) {
        std::vector<size_t> cmd_idx;
        cmd_idx.reserve(buffers_count);
        for(size_t i = 0u; i < buffers_count; ++i) {
            size_t command_buf_idx = -1;
            m_free_cmd_idx.WaitAndPop(command_buf_idx);
            command_buffers[i] = m_command_buffers[command_buf_idx];
            cmd_idx.push_back(command_buf_idx);
        }
        size_t semaphore_idx = -1;
        m_free_smp_idx.WaitAndPop(semaphore_idx);
        VkSemaphore semaphore = m_semaphores[semaphore_idx];
        size_t fence_idx = -1;
        m_free_fnc_idx.WaitAndPop(fence_idx);
        VkFence fence = m_fences[fence_idx];

        CommandBatch result_buffers;
        result_buffers.init(m_device, std::move(command_buffers), semaphore, fence, pool_type, LAST_COMMAND_BUFFER_ID++);
        result_buffers.reset();
        VulkanCommandManager::beginCommandBuffer(result_buffers);
        m_submit_to_buf_id.AddOrUpdateMapping(result_buffers.getId(), cmd_idx);
        m_submit_to_sem_id.AddOrUpdateMapping(result_buffers.getId(), semaphore_idx);
        m_submit_to_fnc_id.AddOrUpdateMapping(result_buffers.getId(), fence_idx);
        return result_buffers;
    }

	if (command_buffer_info) {
        command_buffer_info->commandBufferCount = static_cast<uint32_t>(command_buffers.size());
		VkResult result = vkAllocateCommandBuffers(m_device, command_buffer_info, command_buffers.data());
        if(result != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate command buffers!");
        }
        CommandBatch result_buffers;
        result_buffers.init(m_device, std::move(command_buffers), pool_type, LAST_COMMAND_BUFFER_ID++);
        return result_buffers;
	}

    VkCommandBufferAllocateInfo command_alloc_info{};
    command_alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_alloc_info.commandPool = getCommandPool(pool_type);
    command_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    command_alloc_info.commandBufferCount = static_cast<uint32_t>(command_buffers.size());

	VkResult result = vkAllocateCommandBuffers(m_device, &command_alloc_info, command_buffers.data());
    if(result != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }

    CommandBatch result_buffers;
    result_buffers.init(m_device, std::move(command_buffers), pool_type, LAST_COMMAND_BUFFER_ID++);
    return result_buffers;
}

void VulkanCommandManager::beginCommandBuffer(CommandBatch& command_buffer, size_t index, VkCommandBufferBeginInfo* p_begin_info) {
	if (p_begin_info) {
        if(index == SELECT_ALL_BUFFERS) {
            size_t sz = command_buffer.getCommandBuferCount();
            for (size_t i = 0u; i < sz; ++i) {
                VkResult result = vkBeginCommandBuffer(command_buffer.getCommandBufer(i), p_begin_info);
		        if(result != VK_SUCCESS) {
                    throw std::runtime_error("failed to begin recording commandbuffer!");
                }
            }
        }
        else {
            VkResult result = vkBeginCommandBuffer(command_buffer.getCommandBufer(index), p_begin_info);
		    if(result != VK_SUCCESS) {
                throw std::runtime_error("failed to begin recording commandbuffer!");
            }
        }
		return;
	}

    VkCommandBufferInheritanceInfo inherit_info = {};
    inherit_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
    inherit_info.pNext = NULL;
    inherit_info.renderPass = VK_NULL_HANDLE;
    inherit_info.subpass = 0;
    inherit_info.framebuffer = VK_NULL_HANDLE;
    inherit_info.occlusionQueryEnable = VK_FALSE;
    inherit_info.queryFlags = 0;
    inherit_info.pipelineStatistics = 0;

    VkCommandBufferBeginInfo begin_info = {};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.pNext = nullptr;
    begin_info.flags = 0u;
    begin_info.pInheritanceInfo = &inherit_info;

    if(index == SELECT_ALL_BUFFERS) {
        size_t sz = command_buffer.getCommandBuferCount();
        for (size_t i = 0u; i < sz; ++i) {
            VkResult result = vkBeginCommandBuffer(command_buffer.getCommandBufer(i), &begin_info);
            if(result != VK_SUCCESS) {
               throw std::runtime_error("failed to begin recording commandbuffer!");
            }
        }
    }
    else {
        VkResult result = vkBeginCommandBuffer(command_buffer.getCommandBufer(index), &begin_info);
        if(result != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording commandbuffer!");
        }
    }
}

void VulkanCommandManager::endCommandBuffer(CommandBatch& command_buffer, size_t index) {
    if(index == SELECT_ALL_BUFFERS) {
        size_t sz = command_buffer.getCommandBuferCount();
        for (size_t i = 0u; i < sz; ++i) {
            VkResult result = vkEndCommandBuffer(command_buffer.getCommandBufer(i));
	        if(result != VK_SUCCESS) {
                throw std::runtime_error("failed to record command buffer!");
            }
        }
    }
    else {
	    VkResult result = vkEndCommandBuffer(command_buffer.getCommandBufer(index));
	    if(result != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }
    }
}

void VulkanCommandManager::submitCommandBuffer(CommandBatch& command_buffer, size_t index, VkSubmitInfo* p_submit_info) {
    if(command_buffer.getPoolType() == PoolTypeEnum::TRANSFER) {
        VulkanCommandManager::endCommandBuffer(command_buffer);
        if (p_submit_info) {
            VkResult result = vkQueueSubmit(getQueue(command_buffer.getPoolType()), 1u, p_submit_info, command_buffer.getRenderFence());
            if (result != VK_SUCCESS) {
                throw std::runtime_error("failed to submit draw command buffer!");
            }
            return;
        }
        else {
            VkSubmitInfo submit_info = command_buffer.getSubmitInfo();
            VkResult result = vkQueueSubmit(getQueue(command_buffer.getPoolType()), 1u, &submit_info, command_buffer.getRenderFence());
            if (result != VK_SUCCESS) {
                throw std::runtime_error("failed to submit draw command buffer!");
            }
        }
        m_work_in_progress.Push(command_buffer);
        return;
    }

    if (p_submit_info) {
        VkResult result = vkQueueSubmit(getQueue(command_buffer.getPoolType()), 1u, p_submit_info, command_buffer.getRenderFence());
        if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to submit draw command buffer!");
        }
        return;
    }

    VkSubmitInfo submit_info = command_buffer.getSubmitInfo();
    if(index != SELECT_ALL_BUFFERS) {
        submit_info.commandBufferCount = 1u;
        submit_info.pCommandBuffers = command_buffer.getCommandBuferPtr(index);
    }

    VkResult result = vkQueueSubmit(getQueue(command_buffer.getPoolType()), 1u, &submit_info, command_buffer.getRenderFence());
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }
}

void VulkanCommandManager::wait(PoolTypeEnum pool_type) {
    vkQueueWaitIdle(getQueue(pool_type));
}

void VulkanCommandManager::transitionImageLayout(VkCommandBuffer command_buffer, VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout, uint32_t mip_levels) {
    VkPipelineStageFlags source_stage;
    VkPipelineStageFlags destination_stage;
    
    VkImageMemoryBarrier barrier{};
    if (new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        if (VulkanDevice::hasStencilComponent(format)) {
            barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
    }
    else {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }
    
    if(old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0u;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if(old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        barrier.srcAccessMask = 0u;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destination_stage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    }
    else {
        throw std::invalid_argument("unsupported layout transition!");
    }
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = old_layout;
    barrier.newLayout = new_layout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    //barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0u;
    barrier.subresourceRange.levelCount = mip_levels;
    barrier.subresourceRange.baseArrayLayer = 0u;
    barrier.subresourceRange.layerCount = 1u;
    vkCmdPipelineBarrier(command_buffer, source_stage, destination_stage, 0u, 0u, nullptr, 0u, nullptr, 1u, &barrier);
}

void VulkanCommandManager::copyBufferToImage(VkCommandBuffer command_buffer, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
    VkBufferImageCopy region{};
    region.bufferOffset = 0u;
    region.bufferRowLength = 0u;
    region.bufferImageHeight = 0u;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0u;
    region.imageSubresource.baseArrayLayer = 0u;
    region.imageSubresource.layerCount = 1u;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {width, height, 1};
    vkCmdCopyBufferToImage(command_buffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1u, &region);
}

void VulkanCommandManager::generateMipmaps(VkCommandBuffer command_buffer, VkImage image, VkFormat image_format, int32_t tex_width, uint32_t tex_height, uint32_t mip_levels) {
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.image = image;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseArrayLayer = 0u;
    barrier.subresourceRange.layerCount = 1u;
    barrier.subresourceRange.levelCount = 1u;
    
    int32_t mip_width = tex_width;
    int32_t mip_height = tex_height;
    for (uint32_t i = 1u; i < mip_levels; ++i) {
        barrier.subresourceRange.baseMipLevel = i - 1u;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0u, 0u, nullptr, 0u, nullptr, 1u, &barrier);
        
        VkImageBlit blit{};
        blit.srcOffsets[0] = {0, 0, 0};
        blit.srcOffsets[1] = {mip_width, mip_height, 1};
        blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.srcSubresource.mipLevel = i - 1u;
        blit.srcSubresource.baseArrayLayer = 0u;
        blit.srcSubresource.layerCount = 1u;
        
        blit.dstOffsets[0] = {0, 0, 0};
        blit.dstOffsets[1] = { mip_width > 1 ? mip_width / 2 : 1, mip_height > 1 ? mip_height / 2 : 1, 1 };
        blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.dstSubresource.mipLevel = i;
        blit.dstSubresource.baseArrayLayer = 0u;
        blit.dstSubresource.layerCount = 1u;
        
        vkCmdBlitImage(command_buffer, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1u, &blit, VK_FILTER_LINEAR);
        
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0u, 0u, nullptr, 0u, nullptr, 1u, &barrier);
        
        if (mip_width > 1) {
            mip_width /= 2;
        }
        if (mip_height > 1) {
            mip_height /= 2;
        }
    }
    
    barrier.subresourceRange.baseMipLevel = mip_levels - 1u;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0u, 0u, nullptr, 0u, nullptr, 1u, &barrier);
}

void VulkanCommandManager::copyBuffer(VkCommandBuffer command_buffer, VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size) {
    VkBufferCopy copy_regions{};
    copy_regions.srcOffset = 0u;
    copy_regions.dstOffset = 0u;
    copy_regions.size = size;
    vkCmdCopyBuffer(command_buffer, src_buffer, dst_buffer, 1, &copy_regions);
}

void VulkanCommandManager::createCommandPools() {
    VkCommandPoolCreateInfo gfx_cmd_pool_info{};
    gfx_cmd_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    gfx_cmd_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    gfx_cmd_pool_info.queueFamilyIndex = getQueueFamilyIndices().getFamilyIdx(PoolTypeEnum::GRAPICS).value();
    
    VkResult result = vkCreateCommandPool(m_device, &gfx_cmd_pool_info, nullptr, &m_grapics_cmd_pool);
    if(result != VK_SUCCESS) {
        throw std::runtime_error("failed to create command pool!");
    }
    
    VkCommandPoolCreateInfo transfer_cmd_pool_info{};
    transfer_cmd_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    transfer_cmd_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    transfer_cmd_pool_info.queueFamilyIndex = getQueueFamilyIndices().getFamilyIdx(PoolTypeEnum::TRANSFER).value();
    
    result = vkCreateCommandPool(m_device, &transfer_cmd_pool_info, nullptr, &m_transfer_cmd_pool);
    if(result != VK_SUCCESS) {
        throw std::runtime_error("failed to create command pool!");
    }
}