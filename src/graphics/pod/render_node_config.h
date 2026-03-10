#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <pugixml.hpp>

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class VulkanDevice;

class RenderNodeConfig {
public:
    struct FrameBufferAttachment {
        std::string attachment_name;
        std::string attachment_resource_type;
        std::string attachment_resource_view;
    };

    bool init(const std::shared_ptr<VulkanDevice>& device, const std::string& rg_file_path);
    bool init(const std::shared_ptr<VulkanDevice>& device, const pugi::xml_node& node_data);

    const std::string& getName() const;
    const std::string& getPipelineName() const;
    const std::string& getRenderPassName() const;
    const std::vector<FrameBufferAttachment>& getAttachmentsConfig() const;

    std::shared_ptr<RenderNodeConfig> makeInstance(std::string name) const;

private:

    std::string m_name;
    std::string m_pipeline_name;
    std::string m_render_pass_name;
    std::vector<FrameBufferAttachment> m_attachments;
};