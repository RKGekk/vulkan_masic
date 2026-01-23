#include "pipeline_config.h"

#include "../../tools/string_tools.h"

bool PipelineConfig::init(const pugi::xml_node& pipeline_data) {

    m_name = pipeline_data.attribute("name").as_string();

    pugi::xml_node shaders_node = pipeline_data.child("Shaders");
	if (shaders_node) {
        for (pugi::xml_node shader_node = shaders_node.first_child(); shader_node; shader_node = shader_node.next_sibling()) {
			m_shaders.push_back(shader_node.text().as_string());
		}
    }

    pugi::xml_node input_assembly_node = pipeline_data.child("InputAssembly");
	if (input_assembly_node) {
        m_topology = getPrimitiveTopology(input_assembly_node.child("Topology").text().as_string());
        m_primitive_restart_enable = input_assembly_node.child("PrimitiveRestartEnable").text().as_bool();
    }

    m_tessellation_state_Info = VkPipelineTessellationStateCreateInfo{};
    m_tessellation_state_Info.sType =  VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
    pugi::xml_node tessellation_state_node = pipeline_data.child("RasterizationState");
	if (tessellation_state_node) {
        m_tessellation_state_Info.flags = 0u;
        m_tessellation_state_Info.patchControlPoints = tessellation_state_node.child("PatchControlPoints").text().as_int();
    }

    m_rasterizer_info = VkPipelineRasterizationStateCreateInfo{};
    m_rasterizer_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    pugi::xml_node rasterization_state_node = pipeline_data.child("RasterizationState");
	if (rasterization_state_node) {
        m_rasterizer_info.depthClampEnable = rasterization_state_node.child("DepthClampEnable").text().as_bool();
        m_rasterizer_info.rasterizerDiscardEnable = rasterization_state_node.child("RasterizerDiscardEnable").text().as_bool();
        m_rasterizer_info.polygonMode = getPolygonMode(rasterization_state_node.child("PolygonMode").text().as_string());

        pugi::xml_node cull_mode_flags_node = rasterization_state_node.child("CullMode").child("Flags");
        if(cull_mode_flags_node) {
            for (pugi::xml_node flag_node = cull_mode_flags_node.first_child(); flag_node; flag_node = flag_node.next_sibling()) {
			    m_rasterizer_info.cullMode |= getCullModeFlagBit(flag_node.text().as_string());
		    }
        }

        m_rasterizer_info.frontFace = getFrontFace(rasterization_state_node.child("FrontFace").text().as_string());
        m_rasterizer_info.depthBiasEnable = rasterization_state_node.child("DepthBiasEnable").text().as_bool();
        m_rasterizer_info.depthBiasConstantFactor = rasterization_state_node.child("DepthBiasConstantFactor").text().as_float();
        m_rasterizer_info.depthBiasClamp = rasterization_state_node.child("DepthBiasClamp").text().as_float();
        m_rasterizer_info.depthBiasSlopeFactor = rasterization_state_node.child("DepthBiasSlopeFactor").text().as_float();
        m_rasterizer_info.lineWidth = rasterization_state_node.child("LineWidth").text().as_float();
    }

    m_multisample_info = VkPipelineMultisampleStateCreateInfo{};
    m_multisample_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    pugi::xml_node multisample_state_node = pipeline_data.child("MultisampleState");
    if(multisample_state_node) {
        m_multisample_info.rasterizationSamples = getSampleCountFlagBit(multisample_state_node.child("SampleCount").text().as_string());
        m_multisample_info.sampleShadingEnable = multisample_state_node.child("SampleShadingEnable").text().as_bool();
        m_multisample_info.minSampleShading = multisample_state_node.child("MinSampleShading").text().as_float();
        m_multisample_info.pSampleMask = nullptr;
        m_multisample_info.alphaToCoverageEnable = multisample_state_node.child("alphaToCoverageEnable").text().as_bool();
        m_multisample_info.alphaToOneEnable = multisample_state_node.child("alphaToOneEnable").text().as_bool();
    }

    m_depth_stencil_info = VkPipelineDepthStencilStateCreateInfo{};
    m_depth_stencil_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    pugi::xml_node depth_stencil_state_node = pipeline_data.child("DepthStencilState");
    if(depth_stencil_state_node) {

        pugi::xml_node depth_stencil_flags_node = rasterization_state_node.child("Flags");
        if(depth_stencil_flags_node) {
            for (pugi::xml_node flag_node = depth_stencil_flags_node.first_child(); flag_node; flag_node = flag_node.next_sibling()) {
			    m_depth_stencil_info.flags |= getPipelineDepthStencilStateCreateFlagBit(flag_node.text().as_string());
		    }
        }

        m_depth_stencil_info.depthBoundsTestEnable = depth_stencil_state_node.child("DepthTestEnable").text().as_bool();
        m_depth_stencil_info.depthWriteEnable = depth_stencil_state_node.child("DepthWriteEnable").text().as_bool();
        m_depth_stencil_info.depthCompareOp = getCompareOp(depth_stencil_state_node.child("DepthCompareOp").text().as_string());
        m_depth_stencil_info.depthBoundsTestEnable = depth_stencil_state_node.child("DepthBoundsTestEnable").text().as_bool();
        m_depth_stencil_info.stencilTestEnable = depth_stencil_state_node.child("StencilTestEnable").text().as_bool();

        pugi::xml_node front_stencil_op_node = rasterization_state_node.child("FrontStencilOpState");
        if(front_stencil_op_node) {
            m_depth_stencil_info.front.failOp = getStencilOp(front_stencil_op_node.child("FailOp").text().as_string());
            m_depth_stencil_info.front.passOp = getStencilOp(front_stencil_op_node.child("PassOp").text().as_string());
            m_depth_stencil_info.front.depthFailOp = getStencilOp(front_stencil_op_node.child("DepthFailOp").text().as_string());
            m_depth_stencil_info.front.compareOp = getCompareOp(front_stencil_op_node.child("CompareOp").text().as_string());
            m_depth_stencil_info.front.compareMask = front_stencil_op_node.child("CompareOp").text().as_int();
            m_depth_stencil_info.front.writeMask = front_stencil_op_node.child("WriteMask").text().as_int();
            m_depth_stencil_info.front.reference = front_stencil_op_node.child("Reference").text().as_int();
        }

        pugi::xml_node back_stencil_op_node = rasterization_state_node.child("BackStencilOpState");
        if(back_stencil_op_node) {
            m_depth_stencil_info.back.failOp = getStencilOp(back_stencil_op_node.child("FailOp").text().as_string());
            m_depth_stencil_info.back.passOp = getStencilOp(back_stencil_op_node.child("PassOp").text().as_string());
            m_depth_stencil_info.back.depthFailOp = getStencilOp(back_stencil_op_node.child("DepthFailOp").text().as_string());
            m_depth_stencil_info.back.compareOp = getCompareOp(back_stencil_op_node.child("CompareOp").text().as_string());
            m_depth_stencil_info.back.compareMask = back_stencil_op_node.child("CompareOp").text().as_int();
            m_depth_stencil_info.back.writeMask = back_stencil_op_node.child("WriteMask").text().as_int();
            m_depth_stencil_info.back.reference = back_stencil_op_node.child("Reference").text().as_int();
        }
        
        m_depth_stencil_info.minDepthBounds = rasterization_state_node.child("MinDepthBounds").text().as_float();
        m_depth_stencil_info.maxDepthBounds = rasterization_state_node.child("MaxDepthBounds").text().as_float();
    }

    m_color_blend_info = VkPipelineColorBlendStateCreateInfo{};
    m_color_blend_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    pugi::xml_node color_blend_state_node = pipeline_data.child("ColorBlendState");
    if(color_blend_state_node) {

        pugi::xml_node color_blend_flags_node = color_blend_state_node.child("Flags");
        if(color_blend_flags_node) {
            for (pugi::xml_node flag_node = color_blend_flags_node.first_child(); flag_node; flag_node = flag_node.next_sibling()) {
			    m_color_blend_info.flags |= getPipelineColorBlendStateCreateFlagBit(flag_node.text().as_string());
		    }
        }

        m_color_blend_info.logicOpEnable = color_blend_state_node.child("LogicOpEnable").text().as_bool();
        m_color_blend_info.logicOp = getLogicOp(color_blend_state_node.child("LogicOpEnable").text().as_string());

        pugi::xml_node attachments_node = color_blend_state_node.child("Attachments");
        if(attachments_node) {
            for (pugi::xml_node attachment_node = attachments_node.first_child(); attachment_node; attachment_node = attachment_node.next_sibling()) {
			    VkPipelineColorBlendAttachmentState attachment{};

                attachment.blendEnable = attachments_node.child("BlendEnable").text().as_bool();
                attachment.srcColorBlendFactor = getBlendFactor(attachments_node.child("SrcColorBlendFactor").text().as_string());
                attachment.dstColorBlendFactor = getBlendFactor(attachments_node.child("DstColorBlendFactor").text().as_string());
                attachment.colorBlendOp = getBlendOp(attachments_node.child("ColorBlendOp").text().as_string());
                attachment.srcAlphaBlendFactor = getBlendFactor(attachments_node.child("SrcAlphaBlendFactor").text().as_string());
                attachment.dstAlphaBlendFactor = getBlendFactor(attachments_node.child("DstAlphaBlendFactor").text().as_string());
                attachment.alphaBlendOp = getBlendOp(attachments_node.child("AlphaBlendOp").text().as_string());

                pugi::xml_node color_write_masks_node = color_blend_state_node.child("ColorWriteMask");
                if(color_write_masks_node) {
                    for (pugi::xml_node mask_node = color_write_masks_node.first_child(); mask_node; mask_node = mask_node.next_sibling()) {
		        	    attachment.colorWriteMask |= getColorComponentFlagBit(mask_node.text().as_string());
		            }
                }

                m_color_blend_attachments.push_back(attachment);
		    }
        }

        m_color_blend_info.blendConstants[0] = color_blend_state_node.child("BlendConstant1").text().as_float();
        m_color_blend_info.blendConstants[1] = color_blend_state_node.child("BlendConstant2").text().as_float();
        m_color_blend_info.blendConstants[2] = color_blend_state_node.child("BlendConstant3").text().as_float();
        m_color_blend_info.blendConstants[3] = color_blend_state_node.child("BlendConstant4").text().as_float();
    }

}

bool PipelineConfig::init(const std::string& rg_file_path) {
    pugi::xml_document xml_doc;
	pugi::xml_parse_result parse_res = xml_doc.load_file(rg_file_path.c_str());
	if (!parse_res) { return false;	}

	pugi::xml_node root_node = xml_doc.root();
	if (!root_node) { return false; }
	root_node = root_node.child("RenderGraph");

    pugi::xml_node pipelines_node = root_node.child("Pipelines");
	if (!pipelines_node) return false;
    
	for (pugi::xml_node pipeline_node = pipelines_node.first_child(); pipeline_node; pipeline_node = pipeline_node.next_sibling()) {
	    return init(pipeline_node);
	}
	

    return true;
}