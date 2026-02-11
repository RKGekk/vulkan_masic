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
#include <string>
#include <utility>
#include <vector>

class VulkanUniformBuffer : public VulkanBuffer {
public:

    VulkanUniformBuffer(std::shared_ptr<VulkanDevice> device, std::string name);
    VulkanUniformBuffer(std::shared_ptr<VulkanDevice> device);

    template<typename UniformType>
    bool init(VkMemoryPropertyFlags properties) {
        VulkanBuffer::init(nullptr, sizeof(UniformType), properties, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
        
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