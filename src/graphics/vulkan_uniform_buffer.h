#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "basic_uniform.h"
#include "vulkan_buffer.h"
#include "vulkan_device.h"

#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

template<typename UniformType>
class VulkanUniformBuffer : public IVulkanBuffer {
public:
    bool init(std::shared_ptr<VulkanDevice> device, uint32_t copy_count, VkMemoryPropertyFlags properties) {
        m_device = std::move(device);

        m_uniform_buffers.resize(copy_count);
        m_uniforms_mapped.resize(copy_count);
        m_buffers_info.resize(copy_count);
        m_uniform_size = sizeof(UniformType);
        for (uint32_t i = 0u; i < copy_count; ++i) {
            m_uniform_buffers[i] = m_device->createBuffer(m_uniform_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, properties);
            vkMapMemory(m_device->getDevice(), m_uniform_buffers[i].mem, 0u, m_uniform_size, 0u, &m_uniforms_mapped[i]);
            
            m_buffers_info[i] = VkDescriptorBufferInfo{};
            m_buffers_info[i].buffer = m_uniform_buffers[i].buf;
            m_buffers_info[i].offset = 0u;
            m_buffers_info[i].range = sizeof(UniformType);
        }

        return true;
    }

    void destroy() override {
        for(size_t i = 0u; i < m_uniform_buffers.size(); ++i) {
            vkUnmapMemory(m_device->getDevice(), m_uniform_buffers[i].mem);
            vkDestroyBuffer(m_device->getDevice(), m_uniform_buffers[i].buf, nullptr);
            vkFreeMemory(m_device->getDevice(), m_uniform_buffers[i].mem, nullptr);
        }
    }

    VulkanBuffer getBuffer(uint32_t copy_index) const override {
        return m_uniform_buffers[copy_index];
    }

    void* getMappedBuffer(uint32_t copy_index) const override {
        if (m_uniform_buffers[copy_index].properties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) {
            return m_uniforms_mapped[copy_index];
        }
        throw std::runtime_error("failed map device buffer to host!");
    }

    VkDeviceSize getSize() const override {
        return m_uniform_size;
    }

    VkDescriptorBufferInfo getDescBufferInfo(uint32_t copy_index) const override {
        return m_buffers_info[copy_index];
    }

    void update(const void* src_data, uint32_t copy_index) override {
        if(m_uniform_buffers[copy_index].properties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) {
            memcpy(m_uniforms_mapped[copy_index], src_data, m_uniform_size);
            return;
        }
        VkCommandBuffer command_buffer = m_device->getCommandManager().beginSingleTimeCommands(m_device->getCommandManager().getTransferCommandPool());
        update(command_buffer, src_data, copy_index);
        m_device->getCommandManager().endSingleTimeCommands(command_buffer, m_device->getCommandManager().getTransferQueue(), m_device->getCommandManager().getTransferCommandPool());
    }

    void update(VkCommandBuffer command_buffer, const void* src_data, uint32_t copy_index) override {
        if(m_uniform_buffers[copy_index].properties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) {
            update(src_data, copy_index);
            return;
        }
        VulkanBuffer staging_buffer = m_device->createBuffer(m_uniform_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        void* dst_data;
        vkMapMemory(m_device->getDevice(), staging_buffer.mem, 0, m_uniform_size, 0u, &dst_data);
        memcpy(dst_data, src_data, m_uniform_size);
        vkUnmapMemory(m_device->getDevice(), staging_buffer.mem);

        VkBufferCopy copy_regions{};
        copy_regions.srcOffset = 0u;
        copy_regions.dstOffset = 0u;
        copy_regions.size = m_uniform_size;
        vkCmdCopyBuffer(command_buffer, staging_buffer.buf, m_uniform_buffers[copy_index].buf, 1, &copy_regions);

        VkMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
        VkAccessFlags srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        VkAccessFlags dstAccessMask = VK_ACCESS_UNIFORM_READ_BIT;
    
        vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0u, 1u, &barrier, 0u, nullptr, 0u, nullptr);
    }

private:
    std::shared_ptr<VulkanDevice> m_device;

    VkDeviceSize m_uniform_size;
    std::vector<VulkanBuffer> m_uniform_buffers;
    std::vector<void*> m_uniforms_mapped;

    std::vector<VkDescriptorBufferInfo> m_buffers_info;
};