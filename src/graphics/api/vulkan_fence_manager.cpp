#include "vulkan_fence_manager.h"

#include "vulkan_device.h"
#include "vulkan_fence.h"
#include "../../application.h"

bool VulkanFenceManager::init(std::shared_ptr<VulkanDevice> device) {
    m_device = device;

    return true;
}

void VulkanFenceManager::destroy() {
    for(const auto&[name, fen] : m_fences) {
        fen->destroy();
    }
}

std::shared_ptr<VulkanFence> VulkanFenceManager::getFence(const std::string& new_name) {
    using namespace std::literals;
    if(m_fences.contains(new_name)) return m_fences[new_name];

    std::shared_ptr<VulkanFence> fence = std::make_shared<VulkanFence>();
    fence->init(m_device, true, new_name);

    m_fences[new_name] = fence;

    return fence;
}