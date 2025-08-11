#include "vulkan_command_manager.h"
#include "vulkan_device.h"

bool VulkanCommandManager::init(VkPhysicalDevice physical_device, VkDevice logical_device, VkSurfaceKHR surface) {
    m_device = logical_device;
    m_queue_family_indices.init(physical_device, surface);

    vkGetDeviceQueue(m_device, m_queue_family_indices.getGraphicsFamily().value(), 0u, &m_graphics_queue);
    vkGetDeviceQueue(m_device, m_queue_family_indices.getPresentFamily().value(), 0u, &m_present_queue);
    vkGetDeviceQueue(m_device, m_queue_family_indices.getComputeFamily().value(), 0u, &m_compute_queue);
    vkGetDeviceQueue(m_device, m_queue_family_indices.getTransferFamily().value(), 0u, &m_transfer_queue);

    createCommandPools();
    
    return true;
}

void VulkanCommandManager::destroy() {
    if(m_grapics_cmd_pool != m_transfer_cmd_pool) {
        vkDestroyCommandPool(m_device, m_grapics_cmd_pool, nullptr);
        vkDestroyCommandPool(m_device, m_transfer_cmd_pool, nullptr);
    }
    else {
        vkDestroyCommandPool(m_device, m_grapics_cmd_pool, nullptr);
    }
}

VkCommandPool VulkanCommandManager::getGrapicsCommandPool() const {
    return m_grapics_cmd_pool;
}

VkCommandPool VulkanCommandManager::getTransferCommandPool() const {
    return m_transfer_cmd_pool;
}

VkCommandPool VulkanCommandManager::getComputeCommandPool() const {
    return m_compute_cmd_pool;
}

VkCommandBuffer VulkanCommandManager::allocCommandBuffer(VkCommandPool command_pool, const VkCommandBufferAllocateInfo* command_buffer_info) const {
	if (command_buffer_info) {
        VkCommandBuffer command_buffer;
		VkResult result = vkAllocateCommandBuffers(m_device, command_buffer_info, &command_buffer);
        if(result != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate command buffers!");
        }
        return command_buffer;
	}

	VkCommandBufferAllocateInfo command_alloc_info = {};
	command_alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	command_alloc_info.pNext = NULL;
	command_alloc_info.commandPool = command_pool;
	command_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	command_alloc_info.commandBufferCount = 1u;

    VkCommandBuffer command_buffer;
	VkResult result = vkAllocateCommandBuffers(m_device, &command_alloc_info, &command_buffer);
    if(result != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }
    return command_buffer;
}

std::vector<VkCommandBuffer> VulkanCommandManager::allocCommandBuffer(VkCommandPool command_pool, size_t buffers_count, VkCommandBufferAllocateInfo* command_buffer_info) const {
    std::vector<VkCommandBuffer> command_buffers;
    command_buffers.resize(buffers_count);
	if (command_buffer_info) {
        command_buffer_info->commandBufferCount = static_cast<uint32_t>(command_buffers.size());
        VkCommandBuffer command_buffer;
		VkResult result = vkAllocateCommandBuffers(m_device, command_buffer_info, command_buffers.data());
        if(result != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate command buffers!");
        }
	}

    VkCommandBufferAllocateInfo command_alloc_info{};
    command_alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_alloc_info.commandPool = command_pool;
    command_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    command_alloc_info.commandBufferCount = static_cast<uint32_t>(command_buffers.size());

    VkCommandBuffer command_buffer;
	VkResult result = vkAllocateCommandBuffers(m_device, &command_alloc_info, command_buffers.data());
    if(result != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }

    return command_buffers;
}

void VulkanCommandManager::beginCommandBuffer(VkCommandBuffer command_buffer, VkCommandBufferBeginInfo* p_begin_info) {
	if (p_begin_info) {
		VkResult result = vkBeginCommandBuffer(command_buffer, p_begin_info);
		if(result != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording commandbuffer!");
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

    VkResult result = vkBeginCommandBuffer(command_buffer, &begin_info);
    if(result != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording commandbuffer!");
    }
}

void VulkanCommandManager::endCommandBuffer(VkCommandBuffer command_buffer) {
	VkResult result = vkEndCommandBuffer(command_buffer);
	if(result != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }
}

VkCommandBuffer VulkanCommandManager::beginSingleTimeCommands(VkCommandPool command_pool) const {
    VkCommandBuffer command_buffer = allocCommandBuffer(command_pool);
    
    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    beginCommandBuffer(command_buffer, &begin_info);
    
    return command_buffer;
}

void VulkanCommandManager::endSingleTimeCommands(VkCommandBuffer command_buffer, VkQueue queue, VkCommandPool command_pool) const {
    endCommandBuffer(command_buffer);
    
    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1u;
    submit_info.pCommandBuffers = &command_buffer;
    
    submitCommandBuffer(queue, submit_info);
    vkQueueWaitIdle(queue);
    vkFreeCommandBuffers(m_device, command_pool, 1u, &command_buffer);
}

void VulkanCommandManager::submitCommandBuffer(VkQueue queue, VkCommandBuffer command_buffer, VkFence fence) {
	VkSubmitInfo submit_info = {};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.pNext = nullptr;
	submit_info.waitSemaphoreCount = 0;
	submit_info.pWaitSemaphores = nullptr;
	submit_info.pWaitDstStageMask = nullptr;
	submit_info.commandBufferCount = 1u;
	submit_info.pCommandBuffers = &command_buffer;
	submit_info.signalSemaphoreCount = 0u;
	submit_info.pSignalSemaphores = nullptr;

	submitCommandBuffer(queue, submit_info, fence);
}

void VulkanCommandManager::submitCommandBuffer(VkQueue queue, const std::vector<VkCommandBuffer>& command_buffers, VkFence fence) {
    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.pNext = nullptr;
    submit_info.waitSemaphoreCount = 0u;
    submit_info.pWaitSemaphores = nullptr;
    submit_info.pWaitDstStageMask = nullptr;
    submit_info.commandBufferCount = static_cast<uint32_t>(command_buffers.size());
    submit_info.pCommandBuffers = command_buffers.data();
    submit_info.signalSemaphoreCount = 0u;
    submit_info.pSignalSemaphores = nullptr;

    submitCommandBuffer(queue, submit_info, fence);
}

void VulkanCommandManager::submitCommandBuffer(VkQueue queue, const VkSubmitInfo& submit_info, VkFence fence) {
    VkResult result = vkQueueSubmit(queue, 1, &submit_info, fence);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }
}

const QueueFamilyIndices& VulkanCommandManager::getQueueFamilyIndices() const {
    return m_queue_family_indices;
}
    
VkQueue VulkanCommandManager::getGraphicsQueue() const {
    return m_graphics_queue;
}

VkQueue VulkanCommandManager::getComputeQueue() const {
    return m_compute_queue;
}

VkQueue VulkanCommandManager::getTransferQueue() const {
    return m_transfer_queue;
}
    
VkQueue VulkanCommandManager::getPresentQueue() const {
    return m_present_queue;
}

void VulkanCommandManager::transitionImageLayout(VkCommandPool cmd_pool, VkQueue queue, VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout, uint32_t mip_levels) const {
    VkCommandBuffer command_buffer = beginSingleTimeCommands(cmd_pool);
    
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
    
    endSingleTimeCommands(command_buffer, queue, cmd_pool);
}

void VulkanCommandManager::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout, uint32_t mip_levels) const {
    transitionImageLayout(m_grapics_cmd_pool, m_graphics_queue, image, format, old_layout, new_layout, mip_levels);
}

void VulkanCommandManager::copyBufferToImage(VkCommandPool cmd_pool, VkQueue queue, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) const {
    VkCommandBuffer command_buffer = beginSingleTimeCommands(cmd_pool);
    
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
    
    endSingleTimeCommands(command_buffer, queue, cmd_pool);
}

void VulkanCommandManager::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) const {
    copyBufferToImage(m_grapics_cmd_pool, m_graphics_queue, buffer, image, width, height);
}

void VulkanCommandManager::generateMipmaps(VkCommandPool cmd_pool, VkQueue queue, VkImage image, VkFormat image_format, int32_t tex_width, uint32_t tex_height, uint32_t mip_levels) const {
    VkCommandBuffer command_buffer = beginSingleTimeCommands(cmd_pool);
    
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
    
    endSingleTimeCommands(command_buffer, queue, cmd_pool);
}

void VulkanCommandManager::generateMipmaps(VkImage image, VkFormat image_format, int32_t tex_width, uint32_t tex_height, uint32_t mip_levels) const {
    generateMipmaps(m_grapics_cmd_pool, m_graphics_queue, image, image_format, tex_width, tex_height, mip_levels);
}

void VulkanCommandManager::copyBuffer(VkCommandPool cmd_pool, VkQueue queue, VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size) const {
    VkCommandBuffer command_buffer = beginSingleTimeCommands(cmd_pool);
    
    VkBufferCopy copy_regions{};
    copy_regions.srcOffset = 0u;
    copy_regions.dstOffset = 0u;
    copy_regions.size = size;
    vkCmdCopyBuffer(command_buffer, src_buffer, dst_buffer, 1, &copy_regions);
    
    endSingleTimeCommands(command_buffer, queue, cmd_pool);
}

void VulkanCommandManager::copyBuffer(VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size) const {
    copyBuffer(m_transfer_cmd_pool, m_transfer_queue, src_buffer, dst_buffer, size);
}

void VulkanCommandManager::createCommandPools() {
    VkCommandPoolCreateInfo gfx_cmd_pool_info{};
    gfx_cmd_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    gfx_cmd_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    gfx_cmd_pool_info.queueFamilyIndex = getQueueFamilyIndices().getGraphicsFamily().value();
    
    VkResult result = vkCreateCommandPool(m_device, &gfx_cmd_pool_info, nullptr, &m_grapics_cmd_pool);
    if(result != VK_SUCCESS) {
        throw std::runtime_error("failed to create command pool!");
    }
    
    VkCommandPoolCreateInfo transfer_cmd_pool_info{};
    transfer_cmd_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    transfer_cmd_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    transfer_cmd_pool_info.queueFamilyIndex = getQueueFamilyIndices().getTransferFamily().value();
    
    result = vkCreateCommandPool(m_device, &transfer_cmd_pool_info, nullptr, &m_transfer_cmd_pool);
    if(result != VK_SUCCESS) {
        throw std::runtime_error("failed to create command pool!");
    }
}