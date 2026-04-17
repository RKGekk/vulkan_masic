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
class GraphicsRenderNode;
class VulkanBuffer;
class VulkanImageBuffer;

class ImGUIDrawable : public IVulkanDrawable {
public:
    struct Renderable {
        std::shared_ptr<GraphicsRenderNode> render_node;
        int frame;
    };

    struct PerFrameData {
        std::shared_ptr<VulkanBuffer> uniform_buffer;
        std::shared_ptr<VulkanBuffer> vertex_buffer;
        std::shared_ptr<VulkanBuffer> index_buffer;
        std::vector<ImDrawVert> imgui_vtx;
        std::vector<ImDrawIdx> imgui_idx;
        std::vector<std::shared_ptr<Renderable>> renderables;
    };

    using FrameRenderables = std::vector<std::shared_ptr<Renderable>>;

    bool init(std::shared_ptr<VulkanDevice> device, int max_frames);

    void reset() override;
    void destroy() override;
    void update(const GameTimerDelta& delta, uint32_t image_index) override;

    virtual int order() override;

    void beginFrame();
    void endFrame();

private:
    std::shared_ptr<VulkanImageBuffer> makeFontTexture(std::shared_ptr<VulkanDevice> device, const char* TTF_font_file_name, float fontSizePixels);
    std::shared_ptr<Renderable> makeRenderable(uint32_t image_index);

    std::shared_ptr<VulkanDevice> m_device;
    int m_max_frames;

    std::vector<std::shared_ptr<PerFrameData>> m_per_frame;
    std::shared_ptr<VulkanImageBuffer> m_font_texture;

    ImGuiContext* m_pImgui_ctx;
};