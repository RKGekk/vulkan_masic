#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <functional>
#include <memory>
#include <unordered_map>

#include "../pod/render_resource.h"

class VulkanDevice;
class FramebufferConfig;
class VulkanRenderPass;

class VulkanFramebuffer {
public:
    using LocalName = RenderResource::ResourceName;
    //using LocalAttachMap = std::unordered_map<LocalName, std::shared_ptr<RenderResource>>;

	bool init(std::shared_ptr<VulkanDevice> device, std::shared_ptr<FramebufferConfig> framebuffer_config, std::shared_ptr<VulkanRenderPass> render_pass_ptr, std::function<const std::shared_ptr<RenderResource>&(const LocalName&)> attach_map);
	void destroy();

    VkFramebuffer getFramebuffer() const;
    const std::shared_ptr<FramebufferConfig>& getFramebufferConfig() const;
    const std::vector<VkImageView>& getFramebufferAttachments() const;
    const VkFramebufferCreateInfo& getFramebufferInfo() const;

private:
    std::vector<VkImageView> getAttachments(std::function<const std::shared_ptr<RenderResource>&(const LocalName&)> attach_map) const;

	std::shared_ptr<VulkanDevice> m_device;
	std::shared_ptr<FramebufferConfig> m_framebuffer_config;
    std::shared_ptr<VulkanRenderPass> m_render_pass_ptr;

    std::vector<VkImageView> m_framebuffers_attachments;
    VkFramebufferCreateInfo m_framebuffer_info;
    VkFramebuffer m_frame_buffer;
};