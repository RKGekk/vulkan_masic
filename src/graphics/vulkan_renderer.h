#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "../tools/string_tools.h"
#include "vulkan_device.h"
#include "vulkan_swapchain.h"
#include "vulkan_drawable.h"
#include "vulkan_shader.h"
#include "vulkan_pipeline.h"
#include "basic_vertex.h"
#include "basic_uniform.h"
#include "vulkan_uniform_buffer.h"

class VulkanRenderer {
public:
    bool init(std::shared_ptr<VulkanDevice> device, VkSurfaceKHR surface, GLFWwindow* window, const std::string& texture_path);
    void destroy();
    void recreate();

    const VulkanSwapChain& getSwapchain();
    const std::shared_ptr<VulkanPipeline>& getPipeline();

    void recordCommandBuffer(VkCommandBuffer command_buffer, uint32_t image_index);
    void drawFrame();
    void setFramebufferResized();
    void update_frame(const UniformBufferObject& ubo);
    void addDrawable(std::shared_ptr<VulkanDrawable> drawable);

private:

    void createColorResources();
    void createDepthResources();
    
    VkDescriptorSetLayout createDescSetLayout();
    void createRenderPass();
    VkRenderPass createRenderPass(const SwapchainParams& swapchain_params, VkSubpassDependency pass_dependency);
    
    std::vector<VkFramebuffer> createFramebuffers();
    
    VkSampler createTextureSampler(uint32_t mip_levels);

    void createUniformBuffers();
    void createDescSets(VkDevice device);
    VkDescriptorPool createDescPool(VkDevice device);
    void createSyncObjects();

    std::shared_ptr<VulkanDevice> m_device;

    VulkanSwapChain m_swapchain;

    std::vector<VkFramebuffer> m_out_framebuffers;

    std::vector<ImageBufferAndView> m_out_color_images;
    std::vector<ImageBufferAndView> m_out_depth_images;

    VulkanShader m_vert_shader;
    VulkanShader m_frag_shader;

    VkRenderPass m_render_pass = VK_NULL_HANDLE;

    VkDescriptorSetLayout m_desc_set_layout = VK_NULL_HANDLE;
    std::shared_ptr<VulkanPipeline> m_pipeline;
    VkSampler m_texture_sampler = VK_NULL_HANDLE;

    ImageBufferAndView m_texture_image;

    VkDescriptorPool m_desc_pool = VK_NULL_HANDLE;
    std::vector<VkDescriptorSet> m_desc_sets;
    std::vector<VkCommandBuffer> m_command_buffers;

    std::vector<std::shared_ptr<VulkanDrawable>> m_drawable_list;
    
    std::vector<std::shared_ptr<IVulkanUniformBuffer>> m_uniform_buffers;

    std::vector<VkSemaphore> m_image_available; // signaled when the presentation engine is finished using the image.
    std::vector<VkSemaphore> m_render_finished;
    std::vector<VkFence> m_in_flight_frame; // will be signaled when the command buffers finish

    bool m_framebuffer_resized = false;
};