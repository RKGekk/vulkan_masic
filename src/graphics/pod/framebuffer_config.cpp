#include "framebuffer_config.h"

#include "../api/vulkan_swapchain.h"
#include "../../window_surface.h"
#include "../../tools/string_tools.h"

bool FramebufferConfig::init(const std::shared_ptr<WindowSurface>& window, const SwapchainSupportDetails& swapchain_support_details, const pugi::xml_node& node_data) {
    using namespace std::literals;

    m_name = node_data.attribute("name").as_string();
    m_renderpass_name = node_data.child("RenderPassName").text().as_string();

    pugi::xml_node frame_buffer_flags_node = node_data.child("Falgs");
    if(frame_buffer_flags_node) {
        for (pugi::xml_node flag_node = frame_buffer_flags_node.first_child(); flag_node; flag_node = flag_node.next_sibling()) {
		    m_frame_buffer_flags |= getFramebufferCreateFlag(flag_node.text().as_string());
		}
    }

    pugi::xml_node extent_node = node_data.child("Extent");
    pugi::xml_node offset_node = node_data.child("Offset");
    m_offset_2D.x = offset_node.child("Width").text().as_uint();
    m_offset_2D.y = offset_node.child("Height").text().as_uint();
    std::string extent_source = extent_node.attribute("source").as_string();
    if(extent_source == "auto") m_extent_source = ExtentSource::AUTO;
    else if(extent_source == "as_swapchain") m_extent_source = ExtentSource::AS_SWAPCHAIN;
    else if(extent_source == "exact") m_extent_source = ExtentSource::EXACT;
    pugi::xml_node width_node = extent_node.child("Width");
    if(width_node && m_extent_source == ExtentSource::EXACT) {
        std::string extent_width_str = width_node.text().as_string();
        m_extent_2D.width = static_cast<uint32_t>(std::stoi(extent_width_str));
        m_extent_3D.width = m_extent_2D.width;
        m_extent_2D.height = static_cast<uint32_t>(std::stoi(extent_node.child("Height").text().as_string()));
        m_extent_3D.height = m_extent_2D.height;

        pugi::xml_node depth_node = extent_node.child("Depth");
        if(depth_node) {
            m_extent_3D.depth = static_cast<uint32_t>(std::stoi(depth_node.text().as_string()));
        }
    }
    else if(m_extent_source == ExtentSource::AS_SWAPCHAIN) {
        VkExtent2D swapchain_extent = VulkanSwapChain::chooseSwapExtent(window->GetWindow(), swapchain_support_details.capabilities);
        m_extent_2D.width = swapchain_extent.width;
        m_extent_3D.width = m_extent_2D.width;

        m_extent_2D.height = swapchain_extent.height;
        m_extent_3D.height = m_extent_2D.height;

        m_extent_3D.depth = 1;
    }
    else {
        m_extent_2D = {1u, 1u};
        m_extent_3D = {1u, 1u, 1u};
    }
    m_aspect = ((float)(m_extent_2D.width - m_offset_2D.x)) / ((float)(m_extent_2D.height - m_offset_2D.y));

    pugi::xml_node attachments_node = node_data.child("Attachments");
    if(attachments_node) {
        for (pugi::xml_node attachment_node = attachments_node.first_child(); attachment_node; attachment_node = attachment_node.next_sibling()) {
            std::shared_ptr<FrameBufferAttachment> attachment = std::make_shared<FrameBufferAttachment>();
            attachment->attachment_name = attachment_node.attribute("name").as_string();
            attachment->attachment_name_type_local = attachment_node.attribute("name_type").as_string() == "local"s;
            attachment->attachment_resource_view = attachment_node.child("AttachmentResourceTypeView").text().as_string();

            size_t attach_idx = m_attachments.size();
            m_name_attach_map[attachment->attachment_name] = attach_idx;
            m_attachments.push_back(std::move(attachment));
		}
    }

    return true;
}

const std::string& FramebufferConfig::getName() const {
    return m_name;
}

const std::string& FramebufferConfig::getRenderpassName() const {
    return m_renderpass_name;
}

VkExtent2D FramebufferConfig::getExtent2D() const {
    return m_extent_2D;
}

void FramebufferConfig::setExtent2D(VkExtent2D extent) {
    m_extent_2D = extent;
    m_extent_3D.width = m_extent_2D.width;
    m_extent_3D.height = m_extent_2D.height;

    m_aspect = ((float)m_extent_2D.width) / ((float)m_extent_2D.height);
}

VkExtent3D FramebufferConfig::getExtent3D() const {
    return m_extent_3D;
}

void FramebufferConfig::setExtent3D(VkExtent3D extent) {
    m_extent_3D = extent;
    m_extent_2D.width = m_extent_3D.width;
    m_extent_2D.height = m_extent_3D.height;

    m_aspect = ((float)m_extent_2D.width) / ((float)m_extent_2D.height);
}

VkOffset2D FramebufferConfig::getOffset2D() const {
    return m_offset_2D;
}

VkImageCreateFlags FramebufferConfig::getFlags() const {
    return m_frame_buffer_flags;
}

FramebufferConfig::ExtentSource FramebufferConfig::getExtentSource() const {
    return m_extent_source;
}

const std::vector<std::shared_ptr<FramebufferConfig::FrameBufferAttachment>>& FramebufferConfig::getAttachmentsConfig() const {
    return m_attachments;
}

const std::shared_ptr<FramebufferConfig::FrameBufferAttachment>& FramebufferConfig::getAttachmentData(const std::string& attached_name) const {
    return m_attachments.at(m_name_attach_map.at(attached_name));
}