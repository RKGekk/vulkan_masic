#include "vulkan_semaphore.h"

#include "vulkan_device.h"

bool VulkanSemaphore::init(std::shared_ptr<VulkanDevice> device, VkSemaphore semaphore, bool signalled, bool owned) {
    m_device = device;
	m_semaphore = semaphore;
	m_timeline = 0u;
	m_semaphore_type = VK_SEMAPHORE_TYPE_BINARY_KHR;
	m_signalled = signalled;
	m_owned = owned;
}

bool VulkanSemaphore::init(std::shared_ptr<VulkanDevice> device, VkSemaphore semaphore, uint64_t timeline, bool owned) {
    m_device = device;
	m_semaphore = semaphore;
	m_timeline = timeline;
	m_semaphore_type = VK_SEMAPHORE_TYPE_TIMELINE_KHR;
	m_owned = owned;
}

void VulkanSemaphore::destroy() {
    recycleSemaphore();
}
	
VkSemaphore VulkanSemaphore::getSemaphore() const {
    return m_semaphore;
}

bool VulkanSemaphore::isSignalled() const {
	return m_signalled;
}

uint64_t VulkanSemaphore::getTimelineValue() const {
    return m_timeline;
}

VkSemaphore VulkanSemaphore::consumeSemaphore() {
	VkSemaphore ret = m_semaphore;
	
	m_semaphore = VK_NULL_HANDLE;
	m_signalled = false;
	m_owned = false;
    
	return ret;
}

VkSemaphore VulkanSemaphore::releaseSemaphore() {
	VkSemaphore ret = m_semaphore;

	m_semaphore = VK_NULL_HANDLE;
	m_signalled = false;
	m_owned = false;
    
	return ret;
}

void VulkanSemaphore::waitExternal() {
	m_signalled = false;
}

void VulkanSemaphore::signalExternal() {
	m_signalled = true;
}

void VulkanSemaphore::setSignalIsForeignQueue() {
	m_signal_is_foreign_queue = true;
}

void VulkanSemaphore::setPendingWait() {
	m_pending_wait = true;
}

bool VulkanSemaphore::isPendingWait() const {
	return m_pending_wait;
}

void VulkanSemaphore::setExternalObjectCompatible(VkExternalSemaphoreHandleTypeFlagBits handle_type, VkExternalSemaphoreFeatureFlags features) {
	m_external_compatible_handle_type = handle_type;
	m_external_compatible_features = features;
}

bool VulkanSemaphore::isExternalObjectCompatible() const	{
	return m_external_compatible_features != 0u;
}

VkSemaphoreTypeKHR VulkanSemaphore::getSemaphoreType() const {
	return m_semaphore_type;
}

bool VulkanSemaphore::isProxyTimeline() const {
	return m_proxy_timeline;
}

void VulkanSemaphore::setProxyTimeline() {
	m_proxy_timeline = true;
	m_signalled = false;
}

VkExternalSemaphoreFeatureFlags VulkanSemaphore::getExternalFeatures() const {
	return m_external_compatible_features;
}

VkExternalSemaphoreHandleTypeFlagBits VulkanSemaphore::getExternalHandleType() const {
	return m_external_compatible_handle_type;

	
}

void VulkanSemaphore::setInternalSyncObject() {
	m_internal_sync = true;
}

void VulkanSemaphore::recycleSemaphore() {
	if (!m_owned) {
        return;
    }

	// if (m_internal_sync) {
	// 	if (m_semaphore_type == VK_SEMAPHORE_TYPE_TIMELINE || m_external_compatible_features) {
	// 		m_device->destroy_semaphore_nolock(semaphore);
	// 	}
	// 	else if (is_signalled())
	// 	{
	// 		// We can't just destroy a semaphore if we don't know who signals it (e.g. WSI).
	// 		// Have to consume it by waiting then recycle.
	// 		if (signal_is_foreign_queue)
	// 			device->consume_semaphore_nolock(semaphore);
	// 		else
	// 			device->destroy_semaphore_nolock(semaphore);
	// 	}
	// 	else
	// 		device->recycle_semaphore_nolock(semaphore);
	// }
	// else
	// {
	// 	if (semaphore_type == VK_SEMAPHORE_TYPE_TIMELINE || external_compatible_features)
	// 	{
	// 		device->destroy_semaphore(semaphore);
	// 	}
	// 	else if (is_signalled())
	// 	{
	// 		// We can't just destroy a semaphore if we don't know who signals it (e.g. WSI).
	// 		// Have to consume it by waiting then recycle.
	// 		if (signal_is_foreign_queue)
	// 			device->consume_semaphore(semaphore);
	// 		else
	// 			device->destroy_semaphore(semaphore);
	// 	}
	// 	else
	// 		device->recycle_semaphore(semaphore);
	// }
}