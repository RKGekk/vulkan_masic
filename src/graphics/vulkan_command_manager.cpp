#include "vulkan_command_manager.h"

bool VulkanCommandManager::init(std::shared_ptr<VulkanDevice> device) {
    m_device = std::move(device);
    createCommandPools();
    
    return true;
}

void VulkanCommandManager::destroy() {
    if(m_grapics_cmd_pool != m_transfer_cmd_pool) {
        vkDestroyCommandPool(m_device->getDevice(), m_grapics_cmd_pool, nullptr);
        vkDestroyCommandPool(m_device->getDevice(), m_transfer_cmd_pool, nullptr);
    }
    else {
        vkDestroyCommandPool(m_device->getDevice(), m_grapics_cmd_pool, nullptr);
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

VkCommandBuffer VulkanCommandManager::allocCommandBuffer(VkDevice device, VkCommandPool command_pool, const VkCommandBufferAllocateInfo* command_buffer_info) {
	if (command_buffer_info) {
        VkCommandBuffer command_buffer;
		VkResult result = vkAllocateCommandBuffers(device, command_buffer_info, &command_buffer);
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
	VkResult result = vkAllocateCommandBuffers(device, &command_alloc_info, &command_buffer);
    if(result != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }
    return command_buffer;
}

void VulkanCommandManager::allocCommandBuffer(VkDevice device, VkCommandPool command_pool, std::vector<VkCommandBuffer>& command_buffers, VkCommandBufferAllocateInfo* command_buffer_info) {
	if (command_buffer_info) {
        command_buffer_info->commandBufferCount = static_cast<uint32_t>(command_buffers.size());
        VkCommandBuffer command_buffer;
		VkResult result = vkAllocateCommandBuffers(device, command_buffer_info, command_buffers.data());
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
	VkResult result = vkAllocateCommandBuffers(device, &command_alloc_info, command_buffers.data());
    if(result != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }
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

void VulkanCommandManager::submitCommandBuffer(VkQueue queue, VkCommandBuffer command_buffer, VkFence fence) {
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = nullptr;
	submitInfo.waitSemaphoreCount = 0;
	submitInfo.pWaitSemaphores = nullptr;
	submitInfo.pWaitDstStageMask = nullptr;
	submitInfo.commandBufferCount = 1u;
	submitInfo.pCommandBuffers = &command_buffer;
	submitInfo.signalSemaphoreCount = 0u;
	submitInfo.pSignalSemaphores = nullptr;

	VkResult result = vkQueueSubmit(queue, 1, &submitInfo, fence);
	if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }
}

void VulkanCommandManager::submitCommandBuffer(VkQueue queue, const VkSubmitInfo& submit_info, VkFence fence) {
    VkResult result = vkQueueSubmit(queue, 1, &submit_info, fence);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }
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

    VkResult result = vkQueueSubmit(queue, 1u, &submit_info, fence);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }
}

void VulkanCommandManager::createCommandPools() {
    VkCommandPoolCreateInfo gfx_cmd_pool_info{};
    gfx_cmd_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    gfx_cmd_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    gfx_cmd_pool_info.queueFamilyIndex = m_device->getQueueFamilyIndices().graphics_family.value();
    
    VkResult result = vkCreateCommandPool(m_device->getDevice(), &gfx_cmd_pool_info, nullptr, &m_grapics_cmd_pool);
    if(result != VK_SUCCESS) {
        throw std::runtime_error("failed to create command pool!");
    }
    
    VkCommandPoolCreateInfo transfer_cmd_pool_info{};
    transfer_cmd_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    transfer_cmd_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    transfer_cmd_pool_info.queueFamilyIndex = m_device->getQueueFamilyIndices().transfer_family.value();
    
    result = vkCreateCommandPool(m_device->getDevice(), &transfer_cmd_pool_info, nullptr, &m_transfer_cmd_pool);
    if(result != VK_SUCCESS) {
        throw std::runtime_error("failed to create command pool!");
    }
}