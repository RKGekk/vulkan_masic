#include "vulkan_fence.h"

#include "vulkan_device.h"

bool VulkanFence::init(std::shared_ptr<VulkanDevice> device, VkFence fence) {
    m_device = device;
	m_fence = fence;
	m_timeline_semaphore = VK_NULL_HANDLE;
	m_timeline_value = 0u;
}

bool VulkanFence::init(std::shared_ptr<VulkanDevice> device, uint64_t value, VkSemaphore timeline_semaphore) {
    m_device = device;
	m_fence = VK_NULL_HANDLE;
	m_timeline_semaphore = timeline_semaphore;
	m_timeline_value = value;
}

void VulkanFence::setInternalSyncObject() {
	m_internal_sync = true;
}

void VulkanFence::destroy() {
	if (m_fence != VK_NULL_HANDLE) {
		// if (m_internal_sync) {
		// 	device->reset_fence_nolock(fence, observed_wait);
        // }
		// else {
		// 	device->reset_fence(fence, observed_wait);
        // }
	}
}

VkFence VulkanFence::getFence() const {
	return m_fence;
}

void VulkanFence::wait() {
	// Waiting for the same VkFence in parallel is not allowed, and there seems to be some shenanigans on Intel
	// when waiting for a timeline semaphore in parallel with same value as well.
	std::lock_guard<std::mutex> holder{m_lock};

	if (m_observed_wait) {
		return;
    }

	if (m_timeline_value != 0) {
		VkSemaphoreWaitInfo info = { VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO };
		info.semaphoreCount = 1u;
		info.pSemaphores = &m_timeline_semaphore;
		info.pValues = &m_timeline_value;

        VkResult result = vkWaitSemaphores(m_device->getDevice(), &info, UINT64_MAX);
		if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to wait for timeline semaphore!");
        }
		else {
			m_observed_wait = true;
        }
	}
	else {
        VkResult result = vkWaitForFences(m_device->getDevice(), 1, &m_fence, VK_TRUE, UINT64_MAX);
		if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to wait for fence!");
        }
		else {
			m_observed_wait = true;
        }
	}
}

bool VulkanFence::waitTimeout(uint64_t timeout)
{
	bool ret;
	// Waiting for the same VkFence in parallel is not allowed, and there seems to be some shenanigans on Intel
	// when waiting for a timeline semaphore in parallel with same value as well.
	std::lock_guard<std::mutex> holder{m_lock};

	if (m_observed_wait) {
		return true;
    }

	if (m_timeline_value != 0u) {
		if (timeout == 0u) {
			uint64_t current_value = 0u;
            VkResult result = vkGetSemaphoreCounterValue(m_device->getDevice(), m_timeline_semaphore, &current_value);
			ret = (result == VK_SUCCESS && current_value >= m_timeline_value);
		}
		else {
			VkSemaphoreWaitInfo info = {VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO};
			info.semaphoreCount = 1u;
			info.pSemaphores = &m_timeline_semaphore;
			info.pValues = &m_timeline_value;
            VkResult result = vkWaitSemaphores(m_device->getDevice(), &info, timeout);
			ret = (result == VK_SUCCESS);
		}
	}
	else {
		if (timeout == 0u) {
            VkResult result = vkGetFenceStatus(m_device->getDevice(), m_fence);
			ret = (result == VK_SUCCESS);
        }
		else {
            VkResult result = vkWaitForFences(m_device->getDevice(), 1u, &m_fence, VK_TRUE, timeout);
			ret = (result == VK_SUCCESS);
        }
	}

	if (ret) {
		m_observed_wait = true;
    }
	return ret;
}

// void VulkanFence::operator()(VulkanFence *fence) {
// 	//fence->m_device->handle_pool.fences.free(fence);
// }