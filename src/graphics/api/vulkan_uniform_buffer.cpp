#include "vulkan_uniform_buffer.h"

#include "vulkan_device.h"

VulkanUniformBuffer::VulkanUniformBuffer(std::shared_ptr<VulkanDevice> device, std::string name) : VulkanBuffer(std::move(device), std::move(name)) {}

VulkanUniformBuffer::VulkanUniformBuffer(std::shared_ptr<VulkanDevice> device) : VulkanBuffer(std::move(device)) {}

VkDescriptorBufferInfo VulkanUniformBuffer::getDescBufferInfo() const {
    return m_buffers_info;
}