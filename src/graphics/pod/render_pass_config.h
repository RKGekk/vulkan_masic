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
class VulkanFormatManager;
class FormatConfig;
class VulkanSampler;

class RenderPassConfig {
public:
    bool init(const std::shared_ptr<VulkanDevice>& device, const std::shared_ptr<VulkanFormatManager>& format_manager, const pugi::xml_node& render_pass_data);

    const std::string& getName();
    const VkRenderPassCreateInfo& getRenderPassCreateInfo() const;

    const std::unordered_map<std::string, size_t>& getAttachmentNameToIdxMap() const;
    size_t getAttachmentIdx(const std::string& attachment_name) const;
    const VkAttachmentDescription& getAttachmentDescription(const std::string& attachment_name) const;
    const VkAttachmentDescription& getAttachmentDescription(size_t attachment_idx) const;

    const std::vector<VkClearValue>& getClearValues() const;
    const std::vector<VkClearAttachment>& getClearAttachment() const;

    const std::unordered_map<std::string, std::shared_ptr<FormatConfig>>& getAttachmentNameToFormatMap() const;
    const std::shared_ptr<FormatConfig>& getAttachmentFormat(const std::string& attachment_name) const;

    const std::unordered_map<std::string, size_t> getSubpassNameMap() const;
    size_t getSubpassIdx(const std::string& subpass_name) const;
    const std::string& getSubpassName(size_t subpass_idx) const;
    const VkSubpassDescription& getSubpassDescription(const std::string& subpass_name) const;
    const VkSubpassDescription& getSubpassDescription(size_t subpass_idx) const;

private:
    std::string m_name;
    VkRenderPassCreateFlags m_create_flags;

    std::vector<VkAttachmentDescription> m_attachment_descriptions;
    std::vector<VkClearValue> m_clear_values;
    std::vector<VkClearAttachment> m_clear_attachment;
    std::unordered_map<std::string, size_t> m_attachment_name_to_attach_idx_map;
    std::unordered_map<std::string, std::shared_ptr<FormatConfig>> m_attachment_name_to_format_map;

    std::vector<VkAttachmentReference> m_input_desc_attach_refs;

    std::vector<VkAttachmentReference> m_color_desc_attach_refs;
    std::vector<VkAttachmentReference> m_resolve_desc_attach_refs;
    std::vector<VkAttachmentReference> m_depth_desc_attach_refs;

    std::vector<VkSubpassDescription> m_subpass_descriptions;
    std::vector<std::string> m_subpass_names;
    std::unordered_map<std::string, size_t> m_subpass_name_to_idx_map;

    std::vector<VkSubpassDependency> m_subpass_dependencies_syncs;

    VkRenderPassCreateInfo m_render_pass_create_info;
};