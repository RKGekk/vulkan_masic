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
#include "vulkan_vertex_buffer.h"
#include "vulkan_pipeline.h"

class VulkanDrawable {
public:
    bool init(std::shared_ptr<VulkanDevice> device, std::shared_ptr<VulkanPipeline> pipeline, std::shared_ptr<IVertexBuffer> vertex_buffer);
    void destroy();
    void recordCommandBuffer(VkCommandBuffer command_buffer);

private:
    std::shared_ptr<VulkanDevice> m_device;
    std::shared_ptr<IVertexBuffer> m_vertex_buffer;

	std::shared_ptr<VulkanPipeline> m_pipeline;
};