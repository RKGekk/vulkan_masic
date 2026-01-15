#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "../pod/basic_uniform.h"
#include "vulkan_buffer.h"
#include "vulkan_device.h"
#include "vulkan_command_buffer.h"
#include "../pod/render_resource.h"

#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

class VulkanUniformBuffer : public VulkanBuffer {
public:

    template<typename UniformType>
    bool init(std::shared_ptr<VulkanDevice> device, VkMemoryPropertyFlags properties) {
        VulkanBuffer::init(device, nullptr, sizeof(UniformType), properties, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
        
        m_buffers_info = VkDescriptorBufferInfo{};
        m_buffers_info.buffer = m_buffer;
        m_buffers_info.offset = 0u;
        m_buffers_info.range = sizeof(UniformType);

        return true;
    }

    VkDescriptorBufferInfo getDescBufferInfo() const;

private:
    VkDescriptorBufferInfo m_buffers_info;
};