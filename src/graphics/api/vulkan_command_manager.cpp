#include "vulkan_command_manager.h"
#include "vulkan_device.h"
#include "../pod/render_resource.h"
#include "../../application.h"
#include "../vulkan_renderer.h"

unsigned int LAST_COMMAND_BUFFER_ID = 0u;

bool VulkanCommandManager::init(VkPhysicalDevice physical_device, VkDevice logical_device, VkSurfaceKHR surface, size_t frames_in_flight, size_t number_of_threads) {
    m_device = logical_device;
    m_queue_family_indices.init(physical_device, surface);

    vkGetDeviceQueue(m_device, m_queue_family_indices.getFamilyIdx(PoolTypeEnum::GRAPICS).value(), 0u, &m_graphics_queue);
    vkGetDeviceQueue(m_device, m_queue_family_indices.getFamilyIdx(PoolTypeEnum::COMPUTE).value(), 0u, &m_compute_queue);
    vkGetDeviceQueue(m_device, m_queue_family_indices.getFamilyIdx(PoolTypeEnum::TRANSFER).value(), 0u, &m_transfer_queue);

    m_max_frames = frames_in_flight;
    createCommandPools(frames_in_flight, number_of_threads);

    return true;
}

void VulkanCommandManager::destroy() {
    for(const auto&[pool_type, cmd_pools] : m_command_pools) {
        for(size_t i = 0u; i < cmd_pools.primary_buffer_command_pools.size(); ++i) {
            vkDestroyCommandPool(m_device, m_command_pools[pool_type].primary_buffer_command_pools[i], nullptr);
        }
        for(size_t i = 0u; i < cmd_pools.secondary_buffer_command_pools.size(); ++i) {
            vkDestroyCommandPool(m_device, m_command_pools[pool_type].secondary_buffer_command_pools[i], nullptr);
        }
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

const VulkanCommandManager::CommandBufferPools& VulkanCommandManager::getCommandPool(PoolTypeEnum pool_type) const {
    return m_command_pools.at(pool_type);
}

void VulkanCommandManager::resetGfxPool(size_t frame_id) const {
    vkResetCommandPool(m_device, m_command_pools.at(PoolTypeEnum::GRAPICS).primary_buffer_command_pools.at(frame_id), 0u);
    //vkResetCommandPool(m_device, m_command_pools.at(PoolTypeEnum::GRAPICS).primary_buffer_command_pools.at(frame_id), VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
}

std::shared_ptr<CommandBatch> VulkanCommandManager::allocCommandBufferPtr(PoolTypeEnum pool_type, VkCommandBufferLevel command_level, size_t frame_id, size_t thread_id, size_t buffers_count, std::string name) {
    VkCommandPool command_pool;
    const CommandBufferPools& command_pools = m_command_pools.at(pool_type);
    if(command_level == VK_COMMAND_BUFFER_LEVEL_PRIMARY) {
        command_pool = command_pools.primary_buffer_command_pools[frame_id];
    }
    else {
        command_pool = command_pools.secondary_buffer_command_pools[frame_id * m_max_frames + thread_id];
    }

    std::shared_ptr<CommandBatch::CommandBatchInfo> batch_info = std::make_shared<CommandBatch::CommandBatchInfo>();
    batch_info->pool_type = pool_type;
    batch_info->command_pool = command_pool;
    batch_info->command_level = command_level;
    batch_info->family_index = m_queue_family_indices.getFamilyIdx(pool_type).value();
    batch_info->frame_id = frame_id;
    batch_info->thread_id = thread_id;
    std::shared_ptr<CommandBatch> result_buffers = std::make_shared<CommandBatch>(m_device, std::move(name));
    result_buffers->init(buffers_count, std::move(batch_info));

    ++LAST_COMMAND_BUFFER_ID;
    return result_buffers;
}

void VulkanCommandManager::submitCommandBuffer(std::shared_ptr<CommandBatch>& command_buffer, std::shared_ptr<BatchWaitInfo> wait_info, size_t index, VkSubmitInfo* p_submit_info) {
    if (p_submit_info) {
        VkResult result = vkQueueSubmit(getQueue(command_buffer->getPoolType()), 1u, p_submit_info, wait_info->signal_fence);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to submit draw command buffer!");
        }
        return;
    }

    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.pNext = nullptr;
    submit_info.commandBufferCount = static_cast<uint32_t>(command_buffer->getCommandBuferCount());
    submit_info.pCommandBuffers = command_buffer->getCommandBufers().data();
    if(wait_info) {
        submit_info.waitSemaphoreCount = static_cast<uint32_t>(wait_info->wait_for_semaphores.size());
        submit_info.pWaitSemaphores = wait_info->wait_for_semaphores.data();
        submit_info.pWaitDstStageMask = wait_info->wait_for_stages.data();
        submit_info.signalSemaphoreCount = static_cast<uint32_t>(wait_info->signal_semaphores.size());
        submit_info.pSignalSemaphores = wait_info->signal_semaphores.data();
    }
    
    if(index != CommandBatch::SELECT_ALL_BUFFERS) {
        submit_info.commandBufferCount = 1u;
        submit_info.pCommandBuffers = command_buffer->getCommandBuferPtr(index);
    }

    VkResult result = vkQueueSubmit(getQueue(command_buffer->getPoolType()), 1u, &submit_info, wait_info->signal_fence);
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
    else if(old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destination_stage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    }
    else if(old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destination_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
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

void VulkanCommandManager::copyBufferToImage(VkCommandBuffer command_buffer, VkBuffer buffer, VkImage image, VkExtent3D extent, VkImageLayout image_layout) {
    VkBufferImageCopy region{};
    region.bufferOffset = 0u;
    region.bufferRowLength = 0u;
    region.bufferImageHeight = 0u;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0u;
    region.imageSubresource.baseArrayLayer = 0u;
    region.imageSubresource.layerCount = 1u;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = extent;
    vkCmdCopyBufferToImage(command_buffer, buffer, image, image_layout, 1u, &region);
}

void VulkanCommandManager::generateMipmaps(VkCommandBuffer command_buffer, VkImage image, VkFormat format, VkExtent2D extent, uint32_t mip_levels, VkImageLayout initial_layout, VkImageLayout post_layout) {
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.image = image;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseArrayLayer = 0u;
    barrier.subresourceRange.layerCount = 1u;
    barrier.subresourceRange.levelCount = 1u;

    if(initial_layout != VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        transitionImageLayout(command_buffer, image, format, initial_layout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mip_levels);
    }
    
    int32_t mip_width = extent.width;
    int32_t mip_height = extent.height;
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
        barrier.newLayout = post_layout;
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
    barrier.newLayout = post_layout;
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

void VulkanCommandManager::createCommandPools(size_t frames_in_flight, size_t number_of_threads) {
    VkCommandPoolCreateInfo gfx_primary_cmd_pool_info{};
    gfx_primary_cmd_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    gfx_primary_cmd_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    gfx_primary_cmd_pool_info.queueFamilyIndex = getQueueFamilyIndices().getFamilyIdx(PoolTypeEnum::GRAPICS).value();
    
    m_command_pools[PoolTypeEnum::GRAPICS].primary_buffer_command_pools.resize(frames_in_flight);
    for(size_t i = 0u; i < frames_in_flight; ++i) {
        VkResult result = vkCreateCommandPool(m_device, &gfx_primary_cmd_pool_info, nullptr, &(m_command_pools[PoolTypeEnum::GRAPICS].primary_buffer_command_pools[i]));
        if(result != VK_SUCCESS) {
            throw std::runtime_error("failed to create command pool!");
        }
    }

    VkCommandPoolCreateInfo gfx_secondary_cmd_pool_info{};
    gfx_secondary_cmd_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    gfx_secondary_cmd_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    gfx_secondary_cmd_pool_info.queueFamilyIndex = getQueueFamilyIndices().getFamilyIdx(PoolTypeEnum::GRAPICS).value();

    m_command_pools[PoolTypeEnum::GRAPICS].secondary_buffer_command_pools.resize(number_of_threads * frames_in_flight);
    for(size_t i = 0u; i < number_of_threads * frames_in_flight; ++i) {
        VkResult result = vkCreateCommandPool(m_device, &gfx_secondary_cmd_pool_info, nullptr, &(m_command_pools[PoolTypeEnum::GRAPICS].secondary_buffer_command_pools[i]));
        if(result != VK_SUCCESS) {
            throw std::runtime_error("failed to create command pool!");
        }
    }
    
    VkCommandPoolCreateInfo transfer_cmd_pool_info{};
    transfer_cmd_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    transfer_cmd_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    transfer_cmd_pool_info.queueFamilyIndex = getQueueFamilyIndices().getFamilyIdx(PoolTypeEnum::TRANSFER).value();
    
    m_command_pools[PoolTypeEnum::TRANSFER].primary_buffer_command_pools.resize(number_of_threads * frames_in_flight);
    for(size_t i = 0u; i < number_of_threads * frames_in_flight; ++i) {
        VkResult result = vkCreateCommandPool(m_device, &transfer_cmd_pool_info, nullptr, &(m_command_pools[PoolTypeEnum::TRANSFER].primary_buffer_command_pools[i]));
        if(result != VK_SUCCESS) {
            throw std::runtime_error("failed to create command pool!");
        }
    }
}