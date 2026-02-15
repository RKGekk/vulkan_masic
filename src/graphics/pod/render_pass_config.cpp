#include "render_pass_config.h"

#include "../../tools/string_tools.h"
#include "../api/vulkan_device.h"
#include "../api/vulkan_swapchain.h"

bool RenderPassConfig::init(const std::shared_ptr<VulkanDevice>& device, const std::shared_ptr<VulkanSwapChain>& swapchain, const pugi::xml_node& render_pass_data) {
    using namespace std::literals;

    VkSampleCountFlagBits samples = device->getMsaaSamples();
    VkExtent2D swapchain_extent = swapchain->getSwapchainParams().extent;
    VkImageUsageFlags depth_usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    VkFormat swapchain_color_format = swapchain->getSwapchainParams().surface_format.format;
    VkFormat swapchain_depth_format = device->findDepthFormat(depth_usage, swapchain_extent, 1u, samples);
    bool has_stencil = device->hasStencilComponent(swapchain_depth_format);
    
    m_name = render_pass_data.attribute("name").as_string();

    m_create_flags = {};
    pugi::xml_node flags_node = render_pass_data.child("Flags");
	if (flags_node) {
        for (pugi::xml_node flag_node = flags_node.first_child(); flag_node; flag_node = flag_node.next_sibling()) {
			m_create_flags |= getRenderPassCreateFlag(flag_node.text().as_string());
		}
    }

    pugi::xml_node attachment_descriptions_node = render_pass_data.child("AttachmentDescriptions");
	if (attachment_descriptions_node) {
        for (pugi::xml_node attachment_desc_node = attachment_descriptions_node.first_child(); attachment_desc_node; attachment_desc_node = attachment_desc_node.next_sibling()) {
			VkAttachmentDescription attachment_desc{};

            pugi::xml_node attachment_flags_node = attachment_desc_node.child("AttachmentDescFlags");
	        if (attachment_flags_node) {
                for (pugi::xml_node attachment_flag_node = attachment_flags_node.first_child(); attachment_flag_node; attachment_flag_node = attachment_flag_node.next_sibling()) {
	        		attachment_desc.flags |= getAttachmentDescriptionFlag(attachment_flag_node.text().as_string());
	        	}
            }
            
            pugi::xml_node attachment_format_node = attachment_desc_node.child("Format");
	        if (attachment_format_node) {
                std::string format_str = attachment_format_node.text().as_string();
                if(format_str == "as_swapchain"s) {
                    attachment_desc.format = swapchain_color_format;
                }
                else if(format_str == "find_suitable_depth"s) {
                    attachment_desc.format = swapchain_depth_format;
                }
                else {
                    attachment_desc.format = getFormat(format_str);
                }
            }

            pugi::xml_node attachment_samples_node = attachment_desc_node.child("Samples");
	        if (attachment_samples_node) {
                std::string samples_str = attachment_samples_node.text().as_string();
                if(samples_str == "as_device"s || samples_str == "as_user"s) {
                    attachment_desc.samples = samples;
                }
                else {
                    attachment_desc.samples = getSampleCountFlag(samples_str);
                }
            }

            pugi::xml_node attachment_load_op_node = attachment_desc_node.child("LoadOp");
	        if (attachment_load_op_node) {
                attachment_desc.loadOp = getAttachmentLoadOp(attachment_load_op_node.text().as_string());
            }

            pugi::xml_node attachment_store_op_node = attachment_desc_node.child("StoreOp");
	        if (attachment_store_op_node) {
                attachment_desc.storeOp = getAttachmentStoreOp(attachment_store_op_node.text().as_string());
            }

            pugi::xml_node attachment_stencil_load_op_node = attachment_desc_node.child("StencilLoadOp");
	        if (attachment_stencil_load_op_node) {
                attachment_desc.stencilLoadOp = getAttachmentLoadOp(attachment_stencil_load_op_node.text().as_string());
            }

            pugi::xml_node attachment_stencil_store_op_node = attachment_desc_node.child("StencilStoreOp");
	        if (attachment_stencil_store_op_node) {
                attachment_desc.stencilStoreOp = getAttachmentStoreOp(attachment_stencil_store_op_node.text().as_string());
            }

            pugi::xml_node attachment_initial_layout_node = attachment_desc_node.child("InitialLayout");
	        if (attachment_initial_layout_node) {
                std::string attachment_initial_str = attachment_initial_layout_node.text().as_string();
                if(attachment_initial_str == "auto_depth_stencil_attachment_optimal"s) {
                    VkImageLayout depth_image_layout = has_stencil ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
                    attachment_desc.initialLayout = depth_image_layout;
                }
                else {
                    attachment_desc.initialLayout = getImageLayout(attachment_initial_str);
                }
            }

            pugi::xml_node attachment_final_layout_node = attachment_desc_node.child("FinalLayout");
	        if (attachment_final_layout_node) {
                std::string attachment_final_str = attachment_final_layout_node.text().as_string();
                if(attachment_final_str == "auto_depth_stencil_attachment_optimal"s) {
                    VkImageLayout depth_image_layout = has_stencil ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
                    attachment_desc.finalLayout = depth_image_layout;
                }
                else {
                    attachment_desc.finalLayout = getImageLayout(attachment_final_str);
                }
            }

            std::string attachment_desc_name = attachment_desc_node.attribute("name").as_string();

            size_t attachment_desc_idx = m_attachment_descriptions.size();
            m_attachment_descriptions.push_back(attachment_desc);
            m_attachment_name_to_attach_idx_map.insert({attachment_desc_name, attachment_desc_idx});
		}
    }

    pugi::xml_node subpass_descriptions_node = render_pass_data.child("SubpassDescriptions");
	if (subpass_descriptions_node) {
        for (pugi::xml_node subpass_desc_node = subpass_descriptions_node.first_child(); subpass_desc_node; subpass_desc_node = subpass_desc_node.next_sibling()) {
            VkSubpassDescription subpass_desc{};

            pugi::xml_node subpass_flags_node = subpass_desc_node.child("DescFlags");
	        if (subpass_flags_node) {
                for (pugi::xml_node subpass_flag_node = subpass_flags_node.first_child(); subpass_flag_node; subpass_flag_node = subpass_flag_node.next_sibling()) {
	        		subpass_desc.flags |= getSubpassDescriptionFlag(subpass_flag_node.text().as_string());
	        	}
            }

            pugi::xml_node pipeline_bind_point_node = subpass_desc_node.child("PipelineBindPoint");
	        if (pipeline_bind_point_node) {
                subpass_desc.pipelineBindPoint = getPipelineBindPoint(pipeline_bind_point_node.text().as_string());
            }

            pugi::xml_node input_attachments_node = subpass_desc_node.child("InputAttachments");
	        if (input_attachments_node) {
                for (pugi::xml_node input_attachment_ref_node = input_attachments_node.first_child(); input_attachment_ref_node; input_attachment_ref_node = input_attachment_ref_node.next_sibling()) {
                    VkAttachmentReference attach_ref{};
                    std::string input_attachment_ref_name = input_attachment_ref_node.child("AttachmentName").text().as_string();
                    size_t input_attachment_ref_idx = m_attachment_name_to_attach_idx_map.at(input_attachment_ref_name);
                    attach_ref.attachment = input_attachment_ref_idx;
                    attach_ref.layout = getImageLayout(input_attachment_ref_node.child("Layout").text().as_string());

                    m_input_desc_attach_refs.push_back(attach_ref);
                }
                subpass_desc.inputAttachmentCount = static_cast<uint32_t>(m_input_desc_attach_refs.size());
                subpass_desc.pInputAttachments = m_input_desc_attach_refs.data();
            }

            pugi::xml_node output_attachments_node = subpass_desc_node.child("OutputAttachments");
	        if (output_attachments_node) {
                for (pugi::xml_node output_attachment_ref_node = output_attachments_node.child("AttachmentReferences").first_child(); output_attachment_ref_node; output_attachment_ref_node = output_attachment_ref_node.next_sibling()) {

                    pugi::xml_node color_output_attachments_node = output_attachment_ref_node.child("ColorAttachment");
	                
                    VkAttachmentReference color_attach_ref{};
                    std::string color_attachment_ref_name = color_output_attachments_node.child("AttachmentName").text().as_string();
                    size_t color_attachment_ref_idx = m_attachment_name_to_attach_idx_map.at(color_attachment_ref_name);
                    color_attach_ref.attachment = color_attachment_ref_idx;
                    color_attach_ref.layout = getImageLayout(color_output_attachments_node.child("Layout").text().as_string());
                    m_color_desc_attach_refs.push_back(color_attach_ref);

                    pugi::xml_node resolve_output_attachments_node = output_attachment_ref_node.child("ResolveAttachment");
                    bool msaa_enabled = m_attachment_descriptions.at(color_attachment_ref_idx).samples != VK_SAMPLE_COUNT_1_BIT;

                    if(resolve_output_attachments_node) {
                        VkAttachmentReference resolve_attach_ref{};
                        std::string resolve_attachment_ref_name = resolve_output_attachments_node.child("AttachmentName").text().as_string();
                        size_t resolve_attachment_ref_idx = m_attachment_name_to_attach_idx_map.at(resolve_attachment_ref_name);
                        bool attachment_unused = resolve_output_attachments_node.attribute("attachment_unused").as_bool();
                        resolve_attach_ref.attachment = attachment_unused ? VK_ATTACHMENT_UNUSED : resolve_attachment_ref_idx;
                        resolve_attach_ref.layout = getImageLayout(resolve_output_attachments_node.child("Layout").text().as_string());
                        m_resolve_desc_attach_refs.push_back(resolve_attach_ref);
                    }
	                
                }
                pugi::xml_node depth_stencil_output_attachments_node = output_attachments_node.child("DepthStencilAttachmen");
	            if (depth_stencil_output_attachments_node) {
                    std::string depth_stencil_attachment_ref_name = depth_stencil_output_attachments_node.child("AttachmentName").text().as_string();
                    size_t depth_stencil_attachment_ref_idx = m_attachment_name_to_attach_idx_map.at(depth_stencil_attachment_ref_name);
                    m_depth_desc_attach_ref.attachment = depth_stencil_attachment_ref_idx;
                    m_depth_desc_attach_ref.layout = getImageLayout(depth_stencil_output_attachments_node.child("Layout").text().as_string());
                    subpass_desc.pDepthStencilAttachment = &m_depth_desc_attach_ref;
                }
                else {
                    subpass_desc.pDepthStencilAttachment = nullptr;
                }
                subpass_desc.colorAttachmentCount = static_cast<uint32_t>(m_color_desc_attach_refs.size());
                subpass_desc.pColorAttachments = m_color_desc_attach_refs.data();
                subpass_desc.pResolveAttachments = m_resolve_desc_attach_refs.data();
            }

            m_subpass_descriptions.push_back(subpass_desc);
        }
    }

    pugi::xml_node subpass_dependency_sync_node = render_pass_data.child("SubpassDependenciesSynchronization");
	if (subpass_dependency_sync_node) {
        for (pugi::xml_node subpass_dependency_node = subpass_dependency_sync_node.first_child(); subpass_dependency_node; subpass_dependency_node = subpass_dependency_node.next_sibling()) {
            VkSubpassDependency subpass_dependency_sync{};

            pugi::xml_node src_subpass_node = subpass_dependency_node.child("srcSubpass");
	        if (src_subpass_node) {
                std::string subpass_type_str = src_subpass_node.child("SubpassType").text().as_string(); 
                std::string subpass_name = src_subpass_node.child("SubpassName").text().as_string(); 
                if(subpass_type_str == "Internal"s) {
                    size_t subpass_idx = m_subpass_name_to_idx_map.at(subpass_name);
                    subpass_dependency_sync.srcSubpass = static_cast<uint32_t>(subpass_idx);
                }
                else {
                    subpass_dependency_sync.srcSubpass = VK_SUBPASS_EXTERNAL;
                }
            }

            pugi::xml_node dst_subpass_node = subpass_dependency_node.child("dstSubpass");
	        if (dst_subpass_node) {
                std::string subpass_type_str = dst_subpass_node.child("SubpassType").text().as_string(); 
                std::string subpass_name = dst_subpass_node.child("SubpassName").text().as_string(); 
                if(subpass_type_str == "Internal"s) {
                    size_t subpass_idx = m_subpass_name_to_idx_map.at(subpass_name);
                    subpass_dependency_sync.dstSubpass = static_cast<uint32_t>(subpass_idx);
                }
                else {
                    subpass_dependency_sync.dstSubpass = VK_SUBPASS_EXTERNAL;
                }
            }

            pugi::xml_node src_stage_mask_node = subpass_dependency_node.child("srcStageMask");
	        if (src_stage_mask_node) {
                for (pugi::xml_node mask_node = src_stage_mask_node.first_child(); mask_node; mask_node = mask_node.next_sibling()) {
                    subpass_dependency_sync.srcStageMask |= getPipelineStageFlag(mask_node.text().as_string());
                }
            }

            pugi::xml_node dst_stage_mask_node = subpass_dependency_node.child("dstStageMask");
	        if (dst_stage_mask_node) {
                for (pugi::xml_node mask_node = dst_stage_mask_node.first_child(); mask_node; mask_node = mask_node.next_sibling()) {
                    subpass_dependency_sync.dstStageMask |= getPipelineStageFlag(mask_node.text().as_string());
                }
            }

            pugi::xml_node src_access_mask_node = subpass_dependency_node.child("srcAccessMask");
	        if (src_access_mask_node) {
                for (pugi::xml_node mask_node = src_access_mask_node.first_child(); mask_node; mask_node = mask_node.next_sibling()) {
                    subpass_dependency_sync.srcAccessMask |= getAccessFlag(mask_node.text().as_string());
                }
            }

            pugi::xml_node dst_stage_mask_node = subpass_dependency_node.child("dstAccessMask");
	        if (dst_stage_mask_node) {
                for (pugi::xml_node mask_node = dst_stage_mask_node.first_child(); mask_node; mask_node = mask_node.next_sibling()) {
                    subpass_dependency_sync.dstAccessMask |= getAccessFlag(mask_node.text().as_string());
                }
            }

            pugi::xml_node dependency_flags_node = subpass_dependency_node.child("dependencyFlags");
	        if (dependency_flags_node) {
                for (pugi::xml_node flag_node = dependency_flags_node.first_child(); flag_node; flag_node = flag_node.next_sibling()) {
                    subpass_dependency_sync.dependencyFlags |= getDependencyFlag(flag_node.text().as_string());
                }
            }
            m_subpass_dependencies_syncs.push_back(subpass_dependency_sync);
        }
    }

    m_render_pass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    m_render_pass_create_info.pNext = nullptr;
    m_render_pass_create_info.flags = m_create_flags;
    m_render_pass_create_info.attachmentCount = static_cast<uint32_t>(m_attachment_descriptions.size());
    m_render_pass_create_info.pAttachments = m_attachment_descriptions.data();
    m_render_pass_create_info.subpassCount = static_cast<uint32_t>(m_subpass_descriptions.size());
    m_render_pass_create_info.pSubpasses = m_subpass_descriptions.data();
    m_render_pass_create_info.dependencyCount = static_cast<uint32_t>(m_subpass_dependencies_syncs.size());
    m_render_pass_create_info.pDependencies = m_subpass_dependencies_syncs.data();

    return true;
}

const VkRenderPassCreateInfo& RenderPassConfig::getRenderPassCreateInfo() const {
    return m_render_pass_create_info;
}