#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../api/vulkan_device.h"
#include "vulkan_drawable.h"
#include "../pod/render_resource.h"
#include "../api/vulkan_pipeline.h"
#include "../api/vulkan_shader.h"
#include "../api/vulkan_descriptor.h"

#include <array>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <vector>

class VulkanImageBuffer;
class VulkanBuffer;
struct Managers;
class RenderNode;

class BasicDrawable : public IVulkanDrawable {
public:
    bool init(std::shared_ptr<VulkanDevice> device, std::shared_ptr<Managers>& managers, int max_frames);
    void destroy() override;
    void update(const GameTimerDelta& delta, uint32_t image_index) override;

    virtual int order() override;

private:
    std::shared_ptr<VulkanDevice> m_device;

    std::vector<std::shared_ptr<VulkanBuffer>> m_uniform_buffers;
    std::vector<std::shared_ptr<VulkanBuffer>> m_vertex_buffers;
    std::vector<std::shared_ptr<VulkanBuffer>> m_index_buffers;
    float m_rt_aspect = 1.0f;

    std::shared_ptr<VulkanPipeline> m_pipeline;
    std::vector<std::shared_ptr<RenderNode>> m_render_nodes;

    int m_max_frames;

    std::shared_ptr<VulkanImageBuffer> m_texture;
};