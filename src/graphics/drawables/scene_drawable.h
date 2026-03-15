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

//#include "tiny_gltf.h"

#include "../../scene/nodes/mesh_node.h"
#include "../api/vulkan_device.h"
#include "../drawables/vulkan_drawable.h"
#include "../pod/render_resource.h"
#include "../api/vulkan_pipeline.h"
#include "../api/vulkan_shader.h"
#include "../api/vulkan_descriptor.h"

#include <array>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <vector>

class VulkanBuffer;
class VulkanImageBuffer;
class RenderNode;
struct Managers;

class SceneDrawable : public IVulkanDrawable {
public:
    struct GraphicsPipeline {
        VulkanPipeline pipeline;
        //VulkanPipeline::PipelineCfg pipeline_cfg;
    };

    struct Renderable {
        std::vector<std::shared_ptr<VulkanBuffer>> uniform_buffers;
        std::shared_ptr<VulkanBuffer> vertex_buffer;
        std::shared_ptr<VulkanBuffer> index_buffer;
        std::shared_ptr<VulkanImageBuffer> texture;
        std::vector<std::shared_ptr<RenderNode>> render_nodes;
    };

    bool init(std::shared_ptr<VulkanDevice> device, std::shared_ptr<Managers>& managers, int max_frames);

    void reset() override;
    void destroy() override;
    void update(const GameTimerDelta& delta, uint32_t image_index) override;

    virtual int order() override;

    void addRendeNode(std::shared_ptr<MeshNode> model, std::shared_ptr<Managers>& managers);

private:

    std::shared_ptr<VulkanDevice> m_device;
    float m_rt_aspect = 1.0f;
    int m_max_frames;
    VkExtent2D m_viewport_extent;

    std::vector<std::shared_ptr<MeshNode>> m_mesh_nodes;
    std::vector<std::shared_ptr<Renderable>> m_renderables;
};