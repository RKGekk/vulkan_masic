#include "vulkan_uniform_buffer.h"

VkDescriptorBufferInfo VulkanUniformBuffer::getDescBufferInfo() const {
    return m_buffers_info;
}