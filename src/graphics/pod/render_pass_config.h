#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <pugixml.hpp>

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
#include <unordered_map>

class VulkanDevice;
class VulkanSwapChain;

class RenderPassConfig {
public:
    bool init(const std::shared_ptr<VulkanDevice>& device, const std::shared_ptr<VulkanSwapChain>& swapchain, const pugi::xml_node& render_pass_data);
    void destroy();

private:
    std::string m_name;
    VkRenderPassCreateFlags m_create_flags;

    std::vector<VkAttachmentDescription> m_attachment_descriptions;
    std::unordered_map<std::string, size_t> m_attachment_name_to_attach_idx_map;


    std::vector<VkAttachmentReference> m_input_desc_attach_refs;

    std::vector<VkAttachmentReference> m_color_desc_attach_refs;
    std::vector<VkAttachmentReference> m_resolve_desc_attach_refs;
    VkAttachmentReference m_depth_desc_attach_ref;

    std::vector<VkSubpassDescription> m_subpass_descriptions;
    std::unordered_map<std::string, size_t> m_subpass_name_to_idx_map;
};