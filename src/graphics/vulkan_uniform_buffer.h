#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "basic_uniform.h"
#include "vulkan_device.h"

#include <memory>
#include <utility>

class IVulkanUniformBuffer {
public:
    virtual void destroy() = 0;

    virtual VulkanBuffer getUniformBuffer() const = 0;
    virtual void* getMappedBuffer() const = 0;
    virtual VkDescriptorBufferInfo getDescBufferInfo() const = 0;
    virtual VkDescriptorSetLayoutBinding getDescLayoutBinding() const = 0;
    virtual void update(const void* data) = 0;
};

template<typename UniformType>
class VulkanUniformBuffer : public IVulkanUniformBuffer {
public:
    bool init(std::shared_ptr<VulkanDevice> device, VkDescriptorSetLayoutBinding desc_layout_binding) {
        m_device = std::move(device);
        m_uniform_size = sizeof(UniformType);
        m_uniform_buffer = m_device->createBuffer(m_uniform_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        vkMapMemory(m_device->getDevice(), m_uniform_buffer.mem, 0u, m_uniform_size, 0u, &m_uniform_mapped);

        m_buffer_info = VkDescriptorBufferInfo{};
        m_buffer_info.buffer = m_uniform_buffer.buf;
        m_buffer_info.offset = 0u;
        m_buffer_info.range = sizeof(UniformType);

        m_desc_layout_binding = desc_layout_binding;

        return true;
    }

    void destroy() override {
        vkDestroyBuffer(m_device->getDevice(), m_uniform_buffer.buf, nullptr);
        vkFreeMemory(m_device->getDevice(), m_uniform_buffer.mem, nullptr);
    }

    VulkanBuffer getUniformBuffer() const override {
        return m_uniform_buffer;
    }

    virtual void* getMappedBuffer() const override {
        return m_uniform_mapped;
    }

    virtual VkDescriptorBufferInfo getDescBufferInfo() const override {
        return m_buffer_info;
    }

    virtual VkDescriptorSetLayoutBinding getDescLayoutBinding() const override {
        return m_desc_layout_binding;
    }

    virtual void update(const void* data) override {
        memcpy(m_uniform_mapped, data, m_uniform_size);
    }

private:
    std::shared_ptr<VulkanDevice> m_device;

    VkDeviceSize m_uniform_size;
    VulkanBuffer m_uniform_buffer;
    void* m_uniform_mapped;

    VkDescriptorBufferInfo m_buffer_info;
    VkDescriptorSetLayoutBinding m_desc_layout_binding;
};