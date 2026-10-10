#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <cstdint>
#include <functional>
#include <string>
#include <memory>
#include <unordered_map>
#include <vector>

#include "../pod/render_resource.h"
#include "vulkan_command_buffer.h"

class VulkanDevice;
class BufferConfig;

class VulkanBuffer : public RenderResource, public std::enable_shared_from_this<VulkanBuffer> {
public:
    static const std::shared_ptr<VulkanBuffer> BUFFER_NULL_PTR;

    VulkanBuffer(std::shared_ptr<VulkanDevice> device, std::string name);
    VulkanBuffer(std::shared_ptr<VulkanDevice> device);

    bool init(const void* data, std::shared_ptr<BufferConfig> buffer_config);
    bool init(CommandBatch& command_buffer, const void* data, std::shared_ptr<BufferConfig> buffer_config);

    void destroy() override;

    VkBuffer getBuffer() const;
    VkBuffer* getBufferPtr();
    VkDeviceMemory getMemory() const;
    void* getMappedBuffer() const;
    VkDeviceSize getAlignedSize() const;
    VkDeviceSize getNotAlignedSize() const;

    VkMemoryPropertyFlags getProperties() const;
    VkBufferUsageFlags getUsage() const;

    VkBufferView getBufferView() const;
    VkBufferView getBufferView(const std::string& view_name) const;
    const std::unordered_map<std::string, VkBufferView>& getBufferViewMap() const;

    void update(CommandBatch& command_buffer, const void* src_data, VkDeviceSize buffer_size, VkAccessFlags dstAccessMask);
    void update(CommandBatch& command_buffer, const void* src_data, VkDeviceSize buffer_size);
    void update(const void* src_data, VkDeviceSize buffer_size);

    const ResourceName& getName() const override;
    Type getType() const override;

    void AddUpdateMetadataFn(UpdateMetadataFn fn) override;

protected:
    void setGlobalMemoryUpdateBarier(CommandBatch& command_buffer, VkAccessFlags dstAccessMask);
    void setMemoryUpdateBarier(CommandBatch& command_buffer, VkAccessFlags dstAccessMask);

    std::shared_ptr<VulkanDevice> m_device;
    std::string m_name;

    std::vector<UpdateMetadataFn> m_update_functions;
    VkBuffer m_buffer;
    VkDeviceMemory m_memory;
    void* m_mapped;

    std::unordered_map<std::string, VkBufferView> m_buffer_view_map;
    std::shared_ptr<BufferConfig> m_buffer_config;
};