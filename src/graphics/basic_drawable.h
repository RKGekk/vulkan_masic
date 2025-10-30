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
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <vector>

class BasicDrawable : public IVulkanDrawable {
public:
    bool init(std::shared_ptr<VulkanDevice> device, const RenderTarget& rt);
    void reset(const RenderTarget& rt) override;
    void destroy() override;
    void recordCommandBuffer(CommandBatch& command_buffer, uint32_t frame_index) override;
    void update(const GameTimerDelta& delta, uint32_t image_index) override;

private:
    VkRenderPass createRenderPass(VkFormat color_format, VkFormat depth_format);
    VkRenderPass createRenderPass(VkFormat color_format, VkFormat depth_format, VkSubpassDependency subpass_dependency);
    std::vector<VkFramebuffer> createFramebuffers(const RenderTarget& rt);

    std::shared_ptr<VulkanDevice> m_device;

    std::vector<std::shared_ptr<VulkanUniformBuffer>> m_uniform_buffers;
    std::vector<std::shared_ptr<VertexBuffer>> m_vertex_buffers;
    float m_rt_aspect = 1.0f;
    RenderTargetFormat m_render_target_fmt;

	VulkanPipeline m_pipeline;
    VulkanDescriptor m_descriptor;

    VulkanShader m_vert_shader;
    VulkanShader m_frag_shader;

    VkRenderPass m_render_pass = VK_NULL_HANDLE;
    std::vector<VkFramebuffer> m_out_framebuffers;

    VulkanTexture m_texture;
};