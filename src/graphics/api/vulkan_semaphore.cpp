#include "vulkan_semaphore.h"

#include "vulkan_device.h"
#include "../../application.h"

bool VulkanSemaphore::init(std::shared_ptr<VulkanDevice> device, bool signaled, std::string name) {
	using namespace std::literals;

    m_device = std::move(device);
	m_name = std::move(name);
    m_signaled = signaled;

	VkSemaphoreCreateInfo sem_info{};
    sem_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    if(signaled) {
        sem_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    }
    VkResult result = vkCreateSemaphore(m_device->getDevice(), &sem_info, nullptr, &m_semaphore);
    if(result != VK_SUCCESS) {
        throw std::runtime_error("failed to create semaphore!");
    }

#ifndef NDEBUG
    std::string semaphore_name = "semaphore_"s + m_name;
    auto vkSetDebugUtilsObjectNameEXT = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetInstanceProcAddr(Application::GetInstance().getInstance(), "vkSetDebugUtilsObjectNameEXT");
    VkDebugUtilsObjectNameInfoEXT name_info = {};
    name_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    name_info.objectType = VK_OBJECT_TYPE_SEMAPHORE;
    name_info.objectHandle = (uint64_t)m_semaphore;
    name_info.pObjectName = semaphore_name.c_str();

    vkSetDebugUtilsObjectNameEXT(m_device->getDevice(), &name_info);
#endif
	
	return true;
}

void VulkanSemaphore::setName(std::string new_name) {
	using namespace std::literals;

    m_name = std::move(new_name);

#ifndef NDEBUG
    std::string semaphore_name = "semaphore_"s + m_name;
    auto vkSetDebugUtilsObjectNameEXT = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetInstanceProcAddr(Application::GetInstance().getInstance(), "vkSetDebugUtilsObjectNameEXT");
    VkDebugUtilsObjectNameInfoEXT name_info = {};
    name_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    name_info.objectType = VK_OBJECT_TYPE_SEMAPHORE;
    name_info.objectHandle = (uint64_t)m_semaphore;
    name_info.pObjectName = semaphore_name.c_str();

    vkSetDebugUtilsObjectNameEXT(m_device->getDevice(), &name_info);
#endif
}

void VulkanSemaphore::destroy() {
	vkDestroySemaphore(m_device->getDevice(), m_semaphore, nullptr);
}
	
VkSemaphore VulkanSemaphore::getSemaphore() const {
    return m_semaphore;
}