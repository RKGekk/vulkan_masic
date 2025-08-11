#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <array>
#include <memory>
#include <mutex>
#include <vector>
#include <utility>

#include "vulkan_device.h"

struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 tex_coord;
    
    static const std::vector<VkVertexInputBindingDescription>& getBindingDescriptions() {
        static std::vector<VkVertexInputBindingDescription> binding_desc(1);
        static std::once_flag exe_flag;
        std::call_once(exe_flag, [](){
            binding_desc[0].binding = 0u;
            binding_desc[0].stride = sizeof(Vertex);
            binding_desc[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        });
        
        return binding_desc;
    }
    
    static const std::vector<VkVertexInputAttributeDescription>& getAttributeDescritpions() {
        static std::vector<VkVertexInputAttributeDescription> attribute_desc(3);
        static std::once_flag exe_flag;
        std::call_once(exe_flag, [](){
            attribute_desc[0].binding = 0;
            attribute_desc[0].location = 0;
            attribute_desc[0].format = VK_FORMAT_R32G32B32_SFLOAT;
            attribute_desc[0].offset = offsetof(Vertex, pos);
            
            attribute_desc[1].binding = 0;
            attribute_desc[1].location = 1;
            attribute_desc[1].format = VK_FORMAT_R32G32B32_SFLOAT;
            attribute_desc[1].offset = offsetof(Vertex, color);
            
            attribute_desc[2].binding = 0;
            attribute_desc[2].location = 2;
            attribute_desc[2].format = VK_FORMAT_R32G32_SFLOAT;
            attribute_desc[2].offset = offsetof(Vertex, tex_coord);;
        });
        
        return attribute_desc;
    }

    static VkPipelineVertexInputStateCreateInfo getVertextInputInfo() {
        const std::vector<VkVertexInputBindingDescription>& binding_desc = getBindingDescriptions();
        const std::vector<VkVertexInputAttributeDescription>& attribute_desc = getAttributeDescritpions();
        VkPipelineVertexInputStateCreateInfo vertex_input_info{};
        vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertex_input_info.vertexBindingDescriptionCount = static_cast<uint32_t>(binding_desc.size());
        vertex_input_info.pVertexBindingDescriptions = binding_desc.data();
        vertex_input_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(attribute_desc.size());
        vertex_input_info.pVertexAttributeDescriptions = attribute_desc.data();

        return vertex_input_info;
    }
};

class VulkanDrawable {
public:
    bool init(std::shared_ptr<VulkanDevice> device, const std::vector<Vertex>& vertices, const std::vector<uint16_t>& indices);
    void destroy();

    size_t getIndicesCount() const;
    VulkanBuffer getVertexBuffer() const;
    VulkanBuffer getIndexBuffer() const;

private:
    void createAndTransferVertexBuffer(const std::vector<Vertex>& vertices);
    void createAndTransferIndexBuffer(const std::vector<uint16_t>& indices);

    std::shared_ptr<VulkanDevice> m_device;
    VulkanBuffer m_vertex_buffer;
    VulkanBuffer m_index_buffer;
    size_t m_indices_count = 0u;

	VkVertexInputBindingDescription	m_vertex_input_bind_desc;
	std::vector<VkVertexInputAttributeDescription> vertex_input_attr_descs;
};