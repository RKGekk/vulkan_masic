#include "vulkan_fence_manager.h"

#include "vulkan_device.h"
#include "../../application.h"

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

    return true;
}

void VulkanFenceManager::destroy() {
    for(size_t i = 0u; i < m_fences.size(); ++i) {
        vkDestroyFence(m_device->getDevice(), m_fences[i], nullptr);
    }
}

VkFence VulkanFenceManager::getFence(const std::string& new_name) {
    using namespace std::literals;

    VkFence fence;

    if (m_fences.empty()) {
	    VkFenceCreateInfo fen_info{};
        fen_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fen_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        VkResult result = vkCreateFence(m_device->getDevice(), &fen_info, nullptr, &fence);
        if(result != VK_SUCCESS) {
            throw std::runtime_error("failed to create fence!");
        }
    }
    else {
	    fence = m_fences.back();
	    m_fences.pop_back();
    }

#ifndef NDEBUG
    std::string semaphore_name = "fence_"s + new_name;
    auto vkSetDebugUtilsObjectNameEXT = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetInstanceProcAddr(Application::GetInstance().getInstance(), "vkSetDebugUtilsObjectNameEXT");
    VkDebugUtilsObjectNameInfoEXT name_info = {};
    name_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    name_info.objectType = VK_OBJECT_TYPE_FENCE;
    name_info.objectHandle = (uint64_t)fence;
    name_info.pObjectName = semaphore_name.c_str();

    vkSetDebugUtilsObjectNameEXT(m_device->getDevice(), &name_info);
#endif

    return fence;
}

void VulkanFenceManager::returnFence(VkFence fen) {
    if (fen != VK_NULL_HANDLE) {
	    m_fences.push_back(fen);
    }
}