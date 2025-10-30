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

#include "vulkan_device.h"
#include "vulkan_drawable.h"
#include "render_resource.h"
#include "vulkan_uniform_buffer.h"
#include "vulkan_vertex_buffer.h"
#include "vulkan_pipeline.h"
#include "vulkan_shader.h"
#include "vulkan_descriptor.h"
#include "vulkan_texture.h"

#include <array>
#include <memory>
#include <string>

class ImGUIDrawable : public IVulkanDrawable {
public:
    bool init(std::shared_ptr<VulkanDevice> device, const RenderTarget& rt);

    void reset(const RenderTarget& rt) override;
    void destroy() override;
    void recordCommandBuffer(CommandBatch& command_buffer, uint32_t frame_index) override;
    void update(const GameTimerDelta& delta, uint32_t image_index) override;

private:
    std::vector<VkFramebuffer> createFramebuffers(const RenderTarget& rt);

    std::shared_ptr<VulkanDevice> m_device;

    VulkanPipeline m_pipeline;
    VulkanDescriptor m_descriptor;
    VkRenderPass m_render_pass = VK_NULL_HANDLE;
    std::vector<VkFramebuffer> m_out_framebuffers;

    VulkanShader m_vert_shader;
    VulkanShader m_frag_shader;

    std::shared_ptr<VulkanTexture> m_font_texture;
    std::vector<std::shared_ptr<VertexBuffer>> m_vertex_buffers;
    std::vector<std::shared_ptr<VulkanUniformBuffer>> m_uniform_buffers;

    float m_rt_aspect = 1.0f;
    RenderTargetFormat m_render_target_fmt;
};