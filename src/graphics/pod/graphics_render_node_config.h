#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <pugixml.hpp>

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "render_resource.h"

class VulkanDevice;
class VulkanSampler;
class VulkanResourcesManager;

class GraphicsRenderNodeConfig {
public:
    struct FrameBufferAttachment {
        std::string attachment_name;
        std::string attachment_resource_type;
        std::string attachment_resource_view;
    };

    struct UpdateMetadata {
        std::string name;
        enum class SamplerType { NONE, INLINE, DEFAULT, FROM_IMAGEBUFFER };

        RenderResource::Type resource_type;
        std::string buffer_resource_type_name;

        SamplerType sampler_type;
        std::shared_ptr<VulkanSampler> image_sampler;
        std::string image_resource_type_name;
        std::string image_view_type_name;
        VkImageLayout read_image_layout;
    };

    bool init(const std::shared_ptr<VulkanDevice>& device, const std::shared_ptr<VulkanResourcesManager>& resources_manager, const pugi::xml_node& node_data);

    const std::string& getName() const;
    const std::string& getPipelineName() const;
    const std::string& getRenderPassName() const;
    const std::vector<std::shared_ptr<FrameBufferAttachment>>& getAttachmentsConfig() const;
    const std::shared_ptr<FrameBufferAttachment>& getAttachmentData(const std::string& attached_name) const;
    const std::unordered_map<std::string, std::shared_ptr<UpdateMetadata>>& getBindingsMetadata() const;
    const std::shared_ptr<UpdateMetadata>& getUpdateMetadata(const std::string& binding_name);

    std::shared_ptr<GraphicsRenderNodeConfig> makeInstance(std::string name) const;

private:
    std::string m_name;
    std::string m_pipeline_name;
    std::string m_render_pass_name;
    std::unordered_map<std::string, size_t> m_name_attach_map;
    std::vector<std::shared_ptr<FrameBufferAttachment>> m_attachments;
    std::unordered_map<std::string, std::shared_ptr<UpdateMetadata>> m_bindings_metadata;
};