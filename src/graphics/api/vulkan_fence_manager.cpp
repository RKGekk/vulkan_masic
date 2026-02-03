#include "vulkan_fence_manager.h"

#include "vulkan_device.h"

bool VulkanFenceManager::init(std::shared_ptr<VulkanDevice> device) {
    m_device = device;

    const int NUM_SEM = 32;
    m_fences.resize(NUM_SEM);
    for (uint32_t i = 0u; i < NUM_SEM; ++i) {

        VkFenceCreateInfo fen_info{};
        fen_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fen_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        VkResult result = vkCreateFence(m_device->getDevice(), &fen_info, nullptr, &m_fences[i]);
        if(result != VK_SUCCESS) {
            throw std::runtime_error("failed to create fence!");
        }
    }
}

void VulkanFenceManager::destroy() {
    for(size_t i = 0u; i < m_fences.size(); ++i) {
        vkDestroyFence(m_device->getDevice(), m_fences[i], nullptr);
    }
}

VkFence VulkanFenceManager::getFence() {
    if (m_fences.empty()) {
	    VkFence fence;

	    VkFenceCreateInfo fen_info{};
        fen_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fen_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        VkResult result = vkCreateFence(m_device->getDevice(), &fen_info, nullptr, &fence);
        if(result != VK_SUCCESS) {
            throw std::runtime_error("failed to create fence!");
        }

	    return fence;
    }
    else {
	    VkFence fen = m_fences.back();
	    m_fences.pop_back();
	    return fen;
    }
}

void VulkanFenceManager::returnFence(VkFence fen) {
    if (fen != VK_NULL_HANDLE) {
	    m_fences.push_back(fen);
    }
}