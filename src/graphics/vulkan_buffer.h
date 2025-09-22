#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "vulkan_device.h"
#include "vulkan_command_buffer.h"

#include <cstdint>

class IVulkanBuffer {
public:
    virtual void destroy() = 0;

    virtual VulkanBuffer getBuffer(uint32_t copy_index) const = 0;
    virtual void* getMappedBuffer(uint32_t copy_index) const = 0;
    virtual VkDeviceSize getSize() const = 0;
    virtual VkDescriptorBufferInfo getDescBufferInfo(uint32_t copy_index) const = 0;
    virtual void update(VkCommandBuffer command_buffer, const void* data, uint32_t copy_index) = 0;
};

class IVulkanImageBuffer {
public:
    virtual void destroy() = 0;

    virtual ImageBuffer getImageBuffer() const = 0;
    virtual ImageBufferAndView getImageBufferAndView() const = 0;
    virtual VkSampler getSampler() const = 0;
    virtual VkDeviceSize getSize() const = 0;
    virtual VkDescriptorImageInfo getDescImageInfo() const = 0;
};