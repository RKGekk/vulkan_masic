#include "vulkan_semaphores_manager.h"

#include "vulkan_device.h"
#include "vulkan_semaphore.h"
#include "../../application.h"

bool VulkanSemaphoresManager::init(std::shared_ptr<VulkanDevice> device) {
    m_device = device;

    return true;
}

void VulkanSemaphoresManager::destroy() {
    for(const auto&[name, sem] : m_semaphores) {
        sem->destroy();
    }
    m_semaphores.clear();
}

std::shared_ptr<VulkanSemaphore> VulkanSemaphoresManager::getSemaphore(std::string name) {
    if(m_semaphores.contains(name)) return m_semaphores[name];

    std::shared_ptr<VulkanSemaphore> semaphore = std::make_shared<VulkanSemaphore>();
    semaphore->init(m_device, false, std::move(name));
    m_semaphores[name] = semaphore;

    return semaphore;
}