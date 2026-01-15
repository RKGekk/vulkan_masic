#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <cstdint>
#include <vector>

#include "vulkan_device.h"
#include "vulkan_image_buffer.h"
#include "vulkan_swapchain.h"

class RenderTarget {
public:

    enum class AttachmentLoadOp : int {
        LOAD = 0,
        CLEAR = 1,
        DONT_CARE = 2,
        NONE = 1000400000,
        COUNTER_SELECT = 0x7FFFFFFF
    };

    bool init(std::shared_ptr<VulkanDevice> device, const std::shared_ptr<VulkanSwapChain>& swapchain, int frame_index);
    void destroy();

    VkExtent2D getViewportExtent() const;
    float getAspect() const;

    const VkFormat getColorFormat() const;
    const VkFormat getDepthFormat() const;
    const std::vector<VkImageView>& getAttachments() const;

    VkFramebuffer getFrameBuffer(AttachmentLoadOp load_op = AttachmentLoadOp::COUNTER_SELECT) const;
    VkRenderPass getRenderPass(AttachmentLoadOp load_op = AttachmentLoadOp::COUNTER_SELECT) const;

    AttachmentLoadOp getCurrentLoadOp() const;

    void incExecCounter();
    void resetExecCounter();

private:

    VkFramebuffer createFramebuffer(VkRenderPass render_pass) const;
    VkRenderPass createRenderPass(VkAttachmentLoadOp load_op) const;

    VulkanImageBuffer createResource(VkFormat format, VkImageUsageFlags usage, VkImageAspectFlags image_aspect, VkImageLayout initial_layout = VK_IMAGE_LAYOUT_UNDEFINED) const;

    std::shared_ptr<VulkanDevice> m_device;

    int m_exec_counter;

    VkSampleCountFlagBits m_msaa_samples;
    VkExtent2D m_viewport_extent;
    VkFormat m_color_format;
    VkFormat m_depth_format;
    VkSharingMode m_sharing_mode;
    VulkanImageBuffer m_out_color_image;
    VulkanImageBuffer m_out_depth_image;
    VulkanImageBuffer m_out_swap_chain_image;
    std::vector<VkImageView> m_attachments;
    VkFramebuffer m_clear_frame_buffer;
    VkFramebuffer m_load_frame_buffer;
    VkRenderPass m_clear_render_pass;
    VkRenderPass m_load_render_pass;
};