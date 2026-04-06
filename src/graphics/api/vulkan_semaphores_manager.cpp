#include "vulkan_semaphores_manager.h"

#include "vulkan_device.h"
#include "../../application.h"

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

        m_semaphores.push_back(sem);
    }

    return true;
}

void VulkanSemaphoresManager::destroy() {
    for(VkSemaphore sem : m_semaphores) {
        vkDestroySemaphore(m_device->getDevice(), sem, nullptr);
    }
    m_semaphores = {};
}

VkSemaphore VulkanSemaphoresManager::getSemaphore(const std::string& new_name) {
    VkSemaphore semaphore;

    if (m_semaphores.empty()) {
	    VkSemaphoreCreateInfo sem_info{};
        sem_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        VkResult result = vkCreateSemaphore(m_device->getDevice(), &sem_info, nullptr, &semaphore);
        if(result != VK_SUCCESS) {
            throw std::runtime_error("failed to create semaphore!");
        }
    }
    else {
	    semaphore = m_semaphores.front();
	    m_semaphores.pop_front();
    }

#ifndef NDEBUG
    auto vkSetDebugUtilsObjectNameEXT = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetInstanceProcAddr(Application::GetInstance().getInstance(), "vkSetDebugUtilsObjectNameEXT");
    VkDebugUtilsObjectNameInfoEXT name_info = {};
    name_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    name_info.objectType = VK_OBJECT_TYPE_SEMAPHORE;
    name_info.objectHandle = (uint64_t)semaphore;
    name_info.pObjectName = new_name.c_str();

    vkSetDebugUtilsObjectNameEXT(m_device->getDevice(), &name_info);
#endif

    return semaphore;
}

void VulkanSemaphoresManager::returnSemaphore(VkSemaphore sem) {
	m_semaphores.push_back(sem);
}