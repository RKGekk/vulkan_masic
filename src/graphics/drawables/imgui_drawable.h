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

#include <imgui.h>

#include "../api/vulkan_device.h"
#include "vulkan_drawable.h"
#include "../pod/render_resource.h"
#include "../api/vulkan_pipeline.h"
#include "../api/vulkan_shader.h"
#include "../api/vulkan_descriptor.h"

#include <array>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

struct Managers;
class RenderNode;
class VulkanBuffer;
class VulkanImageBuffer;

class ImGUIDrawable : public IVulkanDrawable {
public:
    bool init(std::shared_ptr<VulkanDevice> device, std::shared_ptr<Managers>& managers, int max_frames);

    void reset() override;
    void destroy() override;
    void update(const GameTimerDelta& delta, uint32_t image_index) override;

    virtual int order() override;

    void beginFrame();
    void endFrame();

private:
    std::shared_ptr<VulkanDevice> m_device;
    int m_max_frames;

    std::shared_ptr<VulkanPipeline> m_pipeline;
    std::vector<std::shared_ptr<RenderNode>> m_render_nodes;
    
    std::shared_ptr<VulkanImageBuffer> m_font_texture;
    std::vector<std::shared_ptr<VulkanBuffer>> m_uniform_buffers;

    std::vector<std::shared_ptr<VulkanBuffer>> m_vertex_buffers;
    std::vector<std::shared_ptr<VulkanBuffer>> m_index_buffers;
    std::vector<std::vector<ImDrawVert>> m_imgui_vtx;
    std::vector<std::vector<ImDrawIdx>> m_imgui_idx;

    ImGuiContext* m_pImgui_ctx;
};