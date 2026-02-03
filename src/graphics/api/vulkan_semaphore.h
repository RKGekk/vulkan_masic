#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <memory>

class VulkanDevice;

class VulkanSemaphore {
public:
    bool init(std::shared_ptr<VulkanDevice> device, VkSemaphore semaphore, bool signalled, bool owned);
    bool init(std::shared_ptr<VulkanDevice> device, VkSemaphore semaphore, uint64_t timeline, bool owned);
    void destroy();
	
	VkSemaphore getSemaphore() const;
    bool isSignalled() const;
	uint64_t getTimelineValue() const;
	VkSemaphore consumeSemaphore();
	VkSemaphore releaseSemaphore();
	void waitExternal();
	void signalExternal();
	void setSignalIsForeignQueue();
	void setPendingWait();
	bool isPendingWait() const;
	void setExternalObjectCompatible(VkExternalSemaphoreHandleTypeFlagBits handle_type, VkExternalSemaphoreFeatureFlags features);
	bool isExternalObjectCompatible() const;
	VkSemaphoreTypeKHR getSemaphoreType() const;
	bool isProxyTimeline() const;
	void setProxyTimeline();
	VkExternalSemaphoreFeatureFlags getExternalFeatures() const;
	VkExternalSemaphoreHandleTypeFlagBits getExternalHandleType() const;
	void setInternalSyncObject();

	VulkanSemaphore &operator=(VulkanSemaphore &&other) noexcept;

	void waitTimeline(uint64_t value);
	bool waitTimelineTimeout(uint64_t value, uint64_t timeout);

	void recycleSemaphore();
	
private:
	std::shared_ptr<VulkanDevice> m_device;
	VkSemaphore m_semaphore = VK_NULL_HANDLE;
	uint64_t m_timeline = 0u;
	VkSemaphoreTypeKHR m_semaphore_type = VK_SEMAPHORE_TYPE_BINARY_KHR;
	bool m_signalled = false;
	bool m_pending_wait = false;
	bool m_owned = false;
	bool m_proxy_timeline = false;
	bool m_signal_is_foreign_queue = false;
    bool m_internal_sync = false;
	VkExternalSemaphoreHandleTypeFlagBits m_external_compatible_handle_type = {};
	VkExternalSemaphoreFeatureFlags m_external_compatible_features = 0;
};