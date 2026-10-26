#include "vulkan_semaphores_manager.h"

#include "vulkan_device.h"
#include "vulkan_semaphore.h"
#include "../../application.h"

bool VulkanSemaphoresManager::init(std::shared_ptr<VulkanDevice> device) {
    m_device = device;

    return true;
}

void VulkanSemaphoresManager::destroy() {
    for(const std::shared_ptr<VulkanSemaphore>& sem : m_semaphores) {
        sem->destroy();
    }
    m_semaphores = {};
}

std::shared_ptr<VulkanSemaphore> VulkanSemaphoresManager::getSemaphore(std::string name) {
    std::shared_ptr<VulkanSemaphore> semaphore;

    if (m_semaphores.empty()) {
	    semaphore = std::make_shared<VulkanSemaphore>();
        semaphore->init(m_device, false, std::move(name));
    }
    else {
	    semaphore = m_semaphores.front();
        semaphore->setName(std::move(name));
	    m_semaphores.pop_front();
    }

    return semaphore;
}

void VulkanSemaphoresManager::returnSemaphore(std::shared_ptr<VulkanSemaphore> sem) {
	m_semaphores.push_back(sem);
}