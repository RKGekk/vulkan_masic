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
#include "../../scene/light_manager.h"
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
#include <unordered_map>
#include <vector>

class VulkanBuffer;
class VulkanImageBuffer;
class GraphicsRenderNode;
class VulkanPushConstant;
class ValueBagNode;

class SceneDrawable : public IVulkanDrawable {
public:

    using RenderableId = size_t;

    struct Renderable {
        std::shared_ptr<MeshNode> mesh_node;
        std::unordered_map<std::string, std::shared_ptr<VulkanBuffer>> uniform_buffers;
        std::shared_ptr<VulkanBuffer> vertex_buffer;
        std::shared_ptr<VulkanBuffer> index_buffer;
        std::shared_ptr<VulkanImageBuffer> texture;
        std::vector<std::shared_ptr<VulkanPushConstant>> const_params;
        std::shared_ptr<GraphicsRenderNode> render_node;
    };

    struct RenderPerFrame {
        std::vector<std::shared_ptr<Renderable>> renderables;
        std::shared_ptr<VulkanBuffer> light_buffer;
    };

    bool init(std::shared_ptr<VulkanDevice> device, int max_frames, std::shared_ptr<LightManager> light_manager);

    void reset() override;
    void destroy() override;
    void update(const GameTimerDelta& delta, uint32_t image_index) override;

    virtual int order() override;

    void addRendeNode(std::shared_ptr<MeshNode> model);

private:
    void updatePushConstants(int frame, RenderableId render_id);
    void updateMVPMatrices(const std::shared_ptr<SceneNode>& scene_node, std::shared_ptr<VulkanBuffer>& uniform_buffer);
    void updateInvMVPMatrices(const std::shared_ptr<SceneNode>& scene_node, std::shared_ptr<VulkanBuffer>& uniform_buffer);
    void updateMaterialProps(const std::shared_ptr<Material>& material, std::shared_ptr<VulkanBuffer>& uniform_buffer);

    std::shared_ptr<VulkanDevice> m_device;
    float m_rt_aspect = 1.0f;
    int m_max_frames;
    VkExtent2D m_viewport_extent;
    std::shared_ptr<LightManager> m_light_manager;

    std::vector<std::shared_ptr<RenderPerFrame>> m_per_frame;
};