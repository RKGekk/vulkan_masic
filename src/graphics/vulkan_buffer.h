#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <cstdint>
#include <memory>

#include "render_resource.h"
#include "vulkan_command_buffer.h"

class VulkanDevice;

class VulkanBuffer : public RenderResource {
public:
    bool init(std::shared_ptr<VulkanDevice> device, const void* data, VkDeviceSize buffer_size, VkMemoryPropertyFlags properties, VkBufferUsageFlags usage);
    bool init(std::shared_ptr<VulkanDevice> device, CommandBatch& command_buffer, const void* data, VkDeviceSize buffer_size, VkMemoryPropertyFlags properties, VkBufferUsageFlags usage);

    void destroy() override;

    VkBuffer getBuffer() const;
    VkDeviceMemory getMemory() const;
    void* getMappedBuffer() const;
    VkDeviceSize getSize() const;

    VkMemoryPropertyFlags getProperties() const;
    VkBufferUsageFlags getUsage() const;

    VkBufferView createBufferView(VkFormat format, VkDeviceSize range, VkDeviceSize offset = 0u) const;

    void update(CommandBatch& command_buffer, const void* src_data, VkDeviceSize buffer_size, VkAccessFlags dstAccessMask);
    void update(CommandBatch& command_buffer, const void* src_data, VkDeviceSize buffer_size);
    void update(const void* src_data, VkDeviceSize buffer_size);

protected:
    void setGlobalMemoryUpdateBarier(CommandBatch& command_buffer, VkAccessFlags dstAccessMask);
    void setMemoryUpdateBarier(CommandBatch& command_buffer, VkAccessFlags dstAccessMask);

    std::shared_ptr<VulkanDevice> m_device;

    VkBuffer m_buffer;
    VkDeviceMemory m_memory;
    void* m_mapped;
    VkDeviceSize m_size;

    VkMemoryPropertyFlags m_properties;
    VkBufferUsageFlags m_usage;
};