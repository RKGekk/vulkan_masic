#include "vulkan_semaphores_manager.h"

#include "vulkan_device.h"

bool VulkanSemaphoresManager::init(std::shared_ptr<VulkanDevice> device) {
    m_device = device;

    const int NUM_SEM = 32;
    for (uint32_t i = 0u; i < NUM_SEM; ++i) {

        VkSemaphore sem;
        VkSemaphoreCreateInfo sem_info{};
        sem_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        VkResult result = vkCreateSemaphore(m_device->getDevice(), &sem_info, nullptr, &sem);
        if(result != VK_SUCCESS) {
            throw std::runtime_error("failed to create semaphore!");
        }

        m_semaphores.push(sem);
    }

    return true;
}

void VulkanSemaphoresManager::destroy() {
    for(size_t i = 0u; i < m_semaphores.size(); ++i) {
        VkSemaphore sem = m_semaphores.back();
        m_semaphores.pop();
        vkDestroySemaphore(m_device->getDevice(), sem, nullptr);
    }
}

VkSemaphore VulkanSemaphoresManager::getSemaphore() {
    if (m_semaphores.empty()) {
	    VkSemaphore semaphore;

	    VkSemaphoreCreateInfo sem_info{};
        sem_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        VkResult result = vkCreateSemaphore(m_device->getDevice(), &sem_info, nullptr, &semaphore);
        if(result != VK_SUCCESS) {
            throw std::runtime_error("failed to create semaphore!");
        }

	    return semaphore;
    }
    else {
	    VkSemaphore sem = m_semaphores.front();
	    m_semaphores.pop();
	    return sem;
    }
}

void VulkanSemaphoresManager::returnSemaphore(VkSemaphore sem) {
    if (sem != VK_NULL_HANDLE) {
	    m_semaphores.push(sem);
    }
}