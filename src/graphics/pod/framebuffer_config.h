#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <pugixml.hpp>

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class VulkanDevice;
class WindowSurface;
class SwapchainSupportDetails;

class FramebufferConfig {
public:
    enum class ExtentSource { AUTO, AS_SWAPCHAIN, EXACT };

    struct FrameBufferAttachment {
        bool attachment_name_type_local;
        std::string attachment_name;
        std::string attachment_resource_view;
    };

    bool init(const std::shared_ptr<WindowSurface>& window, const SwapchainSupportDetails& swapchain_support_details, const pugi::xml_node& format_data);

    const std::string& getName() const;

    VkExtent2D getExtent2D() const;
    void setExtent2D(VkExtent2D extent);
    ExtentSource getExtentSource() const;

    VkExtent3D getExtent3D() const;
    void setExtent3D(VkExtent3D extent);

    VkImageCreateFlags getFlags() const;

    const std::vector<std::shared_ptr<FramebufferConfig::FrameBufferAttachment>>& getAttachmentsConfig() const;
    const std::shared_ptr<FramebufferConfig::FrameBufferAttachment>& getAttachmentData(const std::string& attached_name) const;

private:
    std::string m_name;
    std::string m_renderpass_name;
    VkFramebufferCreateFlags m_frame_buffer_flags;
    ExtentSource m_extent_source;
    VkExtent2D m_extent_2D;
    VkExtent3D m_extent_3D;
    float m_aspect;
    std::unordered_map<std::string, size_t> m_name_attach_map;
    std::vector<std::shared_ptr<FrameBufferAttachment>> m_attachments;
};