#include "vulkan_command_manager.h"
#include "vulkan_device.h"

unsigned int m_last_command_buffer_id = 0u;

bool VulkanCommandManager::init(VkPhysicalDevice physical_device, VkDevice logical_device, VkSurfaceKHR surface) {
    m_device = logical_device;
    m_queue_family_indices.init(physical_device, surface);

    vkGetDeviceQueue(m_device, m_queue_family_indices.getFamilyIdx(PoolTypeEnum::GRAPICS).value(), 0u, &m_graphics_queue);
    vkGetDeviceQueue(m_device, m_queue_family_indices.getFamilyIdx(PoolTypeEnum::COMPUTE).value(), 0u, &m_compute_queue);
    vkGetDeviceQueue(m_device, m_queue_family_indices.getFamilyIdx(PoolTypeEnum::TRANSFER).value(), 0u, &m_transfer_queue);

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

CommandBuffer VulkanCommandManager::allocCommandBuffer(PoolTypeEnum pool_type, const VkCommandBufferAllocateInfo* command_buffer_info) const {
	if (command_buffer_info) {
        VkCommandBuffer vkcommand_buffer;
		VkResult result = vkAllocateCommandBuffers(m_device, command_buffer_info, &vkcommand_buffer);
        if(result != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate command buffers!");
        }
        CommandBuffer command_buffer;
        command_buffer.init(m_device, vkcommand_buffer, pool_type, m_last_command_buffer_id++);
        return command_buffer;
	}

	VkCommandBufferAllocateInfo command_alloc_info = {};
	command_alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	command_alloc_info.pNext = NULL;
	command_alloc_info.commandPool = getCommandPool(pool_type);
	command_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	command_alloc_info.commandBufferCount = 1u;

    VkCommandBuffer vkcommand_buffer;
	VkResult result = vkAllocateCommandBuffers(m_device, &command_alloc_info, &vkcommand_buffer);
    if(result != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }
    CommandBuffer command_buffer;
    command_buffer.init(m_device, vkcommand_buffer, pool_type, m_last_command_buffer_id++);
    return command_buffer;
}

std::vector<CommandBuffer> VulkanCommandManager::allocCommandBuffer(size_t buffers_count, PoolTypeEnum pool_type, VkCommandBufferAllocateInfo* command_buffer_info) const {
	if (command_buffer_info) {
        std::vector<VkCommandBuffer> command_buffers;
        command_buffers.resize(buffers_count);
        command_buffer_info->commandBufferCount = static_cast<uint32_t>(command_buffers.size());
		VkResult result = vkAllocateCommandBuffers(m_device, command_buffer_info, command_buffers.data());
        if(result != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate command buffers!");
        }
        std::vector<CommandBuffer> result_buffers;
        result_buffers.reserve(buffers_count);
        for (VkCommandBuffer buffer : command_buffers) {
            CommandBuffer command_buffer;
            command_buffer.init(m_device, buffer, pool_type, m_last_command_buffer_id++);
            result_buffers.push_back(std::move(command_buffer));
        }
        return result_buffers;
	}

    std::vector<VkCommandBuffer> command_buffers;
    command_buffers.resize(buffers_count);
    VkCommandBufferAllocateInfo command_alloc_info{};
    command_alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_alloc_info.commandPool = getCommandPool(pool_type);
    command_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    command_alloc_info.commandBufferCount = static_cast<uint32_t>(command_buffers.size());

    VkCommandBuffer command_buffer;
	VkResult result = vkAllocateCommandBuffers(m_device, &command_alloc_info, command_buffers.data());
    if(result != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }

    std::vector<CommandBuffer> result_buffers;
    result_buffers.reserve(buffers_count);
    for (VkCommandBuffer buffer : command_buffers) {
        CommandBuffer command_buffer;
        command_buffer.init(m_device, buffer, pool_type, m_last_command_buffer_id++);
        result_buffers.push_back(std::move(command_buffer));
    }
    return result_buffers;
}

void VulkanCommandManager::beginCommandBuffer(CommandBuffer command_buffer, VkCommandBufferBeginInfo* p_begin_info) {
	if (p_begin_info) {
		VkResult result = vkBeginCommandBuffer(command_buffer.getCommandBufer(), p_begin_info);
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

    VkResult result = vkBeginCommandBuffer(command_buffer.getCommandBufer(), &begin_info);
    if(result != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording commandbuffer!");
    }
}

void VulkanCommandManager::endCommandBuffer(CommandBuffer command_buffer) {
	VkResult result = vkEndCommandBuffer(command_buffer.getCommandBufer());
	if(result != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }
}

CommandBuffer VulkanCommandManager::beginSingleTimeCommands(PoolTypeEnum pool_type) const {
    CommandBuffer command_buffer = allocCommandBuffer(pool_type);
    
    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    beginCommandBuffer(command_buffer, &begin_info);
    
    return command_buffer;
}

void VulkanCommandManager::endSingleTimeCommands(CommandBuffer command_buffer) const {
    endCommandBuffer(command_buffer);
    
    VkSemaphore end_semaphores[] = {command_buffer.getInProgressSemaphore()};
    VkCommandBuffer command_buffers[] = {command_buffer.getCommandBufer()};
    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.waitSemaphoreCount = 0u;
    submit_info.pWaitSemaphores = nullptr;
    submit_info.pWaitDstStageMask = nullptr;
    submit_info.commandBufferCount = 1u;
    submit_info.pCommandBuffers = command_buffers;
    submit_info.signalSemaphoreCount = 1u;
    submit_info.pSignalSemaphores = end_semaphores;
    
    submitCommandBuffer(submit_info, command_buffer.getPoolType());
    vkQueueWaitIdle(getQueue(command_buffer.getPoolType()));
    vkFreeCommandBuffers(m_device, getCommandPool(command_buffer.getPoolType()), 1u, command_buffers);
    command_buffer.destroy();
}

void VulkanCommandManager::submitCommandBuffer(CommandBuffer command_buffer, PoolTypeEnum pool_type, VkFence fence) const {
    VkSemaphore end_semaphores[] = {command_buffer.getInProgressSemaphore()};
    VkCommandBuffer command_buffers[] = {command_buffer.getCommandBufer()};
	VkSubmitInfo submit_info = {};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.pNext = nullptr;
	submit_info.waitSemaphoreCount = 0;
	submit_info.pWaitSemaphores = nullptr;
	submit_info.pWaitDstStageMask = nullptr;
	submit_info.commandBufferCount = 1u;
	submit_info.pCommandBuffers = command_buffers;
	submit_info.signalSemaphoreCount = 1u;
	submit_info.pSignalSemaphores = end_semaphores;

	submitCommandBuffer(submit_info, pool_type, fence);
}

void VulkanCommandManager::submitCommandBuffer(const std::vector<CommandBuffer>& command_buffers, PoolTypeEnum pool_type, VkFence fence) const {
    std::vector<VkSemaphore> end_semaphores;
    end_semaphores.reserve(command_buffers.size());
    std::vector<VkCommandBuffer> vkcommand_buffers;
    vkcommand_buffers.reserve(command_buffers.size());
    for(const CommandBuffer& buffer: command_buffers) {
        vkcommand_buffers.push_back(buffer.getCommandBufer());
        end_semaphores.push_back(buffer.getInProgressSemaphore());
    }

    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.pNext = nullptr;
    submit_info.waitSemaphoreCount = 0u;
    submit_info.pWaitSemaphores = nullptr;
    submit_info.pWaitDstStageMask = nullptr;
    submit_info.commandBufferCount = static_cast<uint32_t>(command_buffers.size());
    submit_info.pCommandBuffers = vkcommand_buffers.data();
    submit_info.signalSemaphoreCount = static_cast<uint32_t>(command_buffers.size());
    submit_info.pSignalSemaphores = end_semaphores.data();

    submitCommandBuffer(submit_info, pool_type, fence);
}

void VulkanCommandManager::submitCommandBuffer(const VkSubmitInfo& submit_info, PoolTypeEnum pool_type, VkFence fence) const {
    VkResult result = vkQueueSubmit(getQueue(pool_type), 1, &submit_info, fence);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }
}

void VulkanCommandManager::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout, uint32_t mip_levels, PoolTypeEnum pool_type) const {
    CommandBuffer command_buffer = beginSingleTimeCommands(pool_type);
    
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
    vkCmdPipelineBarrier(command_buffer.getCommandBufer(), source_stage, destination_stage, 0u, 0u, nullptr, 0u, nullptr, 1u, &barrier);
    
    endSingleTimeCommands(command_buffer);
}

void VulkanCommandManager::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, PoolTypeEnum pool_type) const {
    CommandBuffer command_buffer = beginSingleTimeCommands(pool_type);
    
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
    vkCmdCopyBufferToImage(command_buffer.getCommandBufer(), buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1u, &region);
    
    endSingleTimeCommands(command_buffer);
}

void VulkanCommandManager::generateMipmaps(VkImage image, VkFormat image_format, int32_t tex_width, uint32_t tex_height, uint32_t mip_levels, PoolTypeEnum pool_type) const {
    CommandBuffer command_buffer = beginSingleTimeCommands(pool_type);
    
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
        vkCmdPipelineBarrier(command_buffer.getCommandBufer(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0u, 0u, nullptr, 0u, nullptr, 1u, &barrier);
        
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
        
        vkCmdBlitImage(command_buffer.getCommandBufer(), image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1u, &blit, VK_FILTER_LINEAR);
        
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        vkCmdPipelineBarrier(command_buffer.getCommandBufer(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0u, 0u, nullptr, 0u, nullptr, 1u, &barrier);
        
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

    vkCmdPipelineBarrier(command_buffer.getCommandBufer(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0u, 0u, nullptr, 0u, nullptr, 1u, &barrier);
    
    endSingleTimeCommands(command_buffer);
}

void VulkanCommandManager::copyBuffer(VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size, PoolTypeEnum pool_type) const {
    CommandBuffer command_buffer = beginSingleTimeCommands(pool_type);
    
    VkBufferCopy copy_regions{};
    copy_regions.srcOffset = 0u;
    copy_regions.dstOffset = 0u;
    copy_regions.size = size;
    vkCmdCopyBuffer(command_buffer.getCommandBufer(), src_buffer, dst_buffer, 1, &copy_regions);
    
    endSingleTimeCommands(command_buffer);
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