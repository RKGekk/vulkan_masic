#include "vulkan_fence.h"

#include "vulkan_device.h"
#include "../../application.h"

bool VulkanFence::init(std::shared_ptr<VulkanDevice> device, bool signaled, std::string name) {
    m_device = device;
	
	VkFenceCreateInfo fen_info{};
    fen_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fen_info.flags = signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0u;
    VkResult result = vkCreateFence(m_device->getDevice(), &fen_info, nullptr, &m_fence);
    if(result != VK_SUCCESS) {
        throw std::runtime_error("failed to create fence!");
    }

	return true;
}

void VulkanFence::destroy() {
	vkDestroyFence(m_device->getDevice(), m_fence, nullptr);
}

VkFence VulkanFence::getFence() const {
	return m_fence;
}

const VkFence* VulkanFence::getFencePtr() const {
    return &m_fence;
}

void VulkanFence::wait() const {
	
	VkResult result = vkWaitForFences(m_device->getDevice(), 1u, &m_fence, VK_TRUE, UINT64_MAX);
	if(result != VK_SUCCESS) {
    	throw std::runtime_error("Failed to wait for fence!");
    }
}

void VulkanFence::reset() const {
    VkResult result = vkResetFences(m_device->getDevice(), 1u, &m_fence);
    if(result != VK_SUCCESS) {
    	throw std::runtime_error("Failed to wait for fence!");
    }
}