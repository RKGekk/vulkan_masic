#include "graphics_render_node.h"

#include "graphics_render_node_config.h"
#include "render_graph.h"
#include "render_node.h"
#include "image_buffer_config.h"
#include "format_config.h"
#include "render_pass_config.h"
#include "../../application.h"
#include "../vulkan_renderer.h"
#include "../api/vulkan_pipelines_manager.h"
#include "../api/vulkan_render_pass.h"
#include "../api/vulkan_descriptors_manager.h"
#include "../api/vulkan_framebuffer.h"
#include "../api/vulkan_resources_manager.h"

#include <utility>

bool GraphicsRenderNode::init(std::shared_ptr<VulkanDevice> device, const std::string& node_config_name, std::weak_ptr<RenderGraph> render_graph) {
    m_device = std::move(device);
    m_render_graph = std::move(render_graph);
    m_node_config = m_render_graph.lock()->getGraphicsRenderNodeConfig(node_config_name);
    m_pipeline = m_node_config->getPipeline();

    return true;
}

void GraphicsRenderNode::destroy() {
    m_frame_buffer->destroy();
}

void GraphicsRenderNode::render(CommandBatch& command_buffer, unsigned image_index) {
    // return all RenderResources to initial state
    
    TransitionResourcesToProperState(command_buffer);

    VkViewport view_port = m_node_config->getViewport();
    VkRect2D scissor = m_node_config->getScissor();
        
    vkCmdSetViewport(command_buffer.getCommandBufer(), 0u, 1u, &view_port);
    vkCmdSetScissor(command_buffer.getCommandBufer(), 0u, 1u, &scissor);
        
    for(const auto&[slot, desc] : m_descs) {
        vkCmdBindDescriptorSets(
            command_buffer.getCommandBufer(), // commandBuffer
            VK_PIPELINE_BIND_POINT_GRAPHICS, // pipelineBindPoint
            m_pipeline->getPipelineLayout(), // pipeline layout
            slot, // first set
            1, // descriptor set count
            desc->getDescriptorSetPtr(), // descriptor sets pointer
            0, // dynamic offset count
            nullptr // dynamic offsets pointer
        );
    }

    for (const VertexFormat& vf : m_pipeline->getShader(VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT)->getShaderSignature()->getInputAttributes()) {
        const std::string& vertex_buffer_name = vf.getVertexBufferBindingName();
        std::shared_ptr<VulkanBuffer> vertex_buffer = getReadAttachedBufferResource(vertex_buffer_name);
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(
            command_buffer.getCommandBufer(), // commandBuffer
            vf.getBindingNum(), // firstBinding
            1, // bindingCount
            vertex_buffer->getBufferPtr(), // buffers pointer
            offsets // offsets pointer
        );
    }
    const std::string& index_buffer_name = m_pipeline->getShader(VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT)->getShaderSignature()->getVertexFormat().getIndexBufferBindingName();
    std::shared_ptr<VulkanBuffer> index_buffer = getReadAttachedBufferResource(index_buffer_name);
    vkCmdBindIndexBuffer(
        command_buffer.getCommandBufer(), // commandBuffer
        index_buffer->getBuffer(), // buffer
        0u, // offset
        m_pipeline->getShader(VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT)->getShaderSignature()->getVertexFormat().getIndexType() // indexType
    );
        
    if(m_node_config->getIndexCountType() == GraphicsRenderNodeConfig::IndexCountType::ALL) {
        uint32_t index_count = index_buffer->getNotAlignedSize() / m_pipeline->getShader(VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT)->getShaderSignature()->getVertexFormat().getIndexTypeBytesCount();
        vkCmdDrawIndexed(
            command_buffer.getCommandBufer(), // commandBuffer
            index_count, // indexCount
            1u, // instanceCount
            0u, // firstIndex
            0u, // vertexOffset
            0u // firstInstance
        );
    }
    else {
        uint32_t index_count = m_node_config->getIndexCount();
        vkCmdDrawIndexed(
            command_buffer.getCommandBufer(), // commandBuffer
            index_count, // indexCount
            1u, // instanceCount
            m_node_config->getFirstIndex(), // firstIndex
            m_node_config->getVertexOffset(), // vertexOffset
            0u // firstInstance
        );
    }
}

void GraphicsRenderNode::finishRenderNode() {
    VulkanRenderer& renderer = Application::GetRenderer();

    VkExtent2D extent = getWrittenAttachedImageResource(m_node_config->getAttachmentsConfig().front()->attachment_name)->getImageConfig()->getFormat()->getExtent2D();
    if(m_node_config->getViewportSource() == GraphicsRenderNodeConfig::ExtentSource::AUTO) {
        VkViewport viewport = m_node_config->getViewport();
        viewport.width = extent.width;
        viewport.height = extent.height;
        m_node_config->setViewport(viewport);
    }
    if(m_node_config->getScissorSource() == GraphicsRenderNodeConfig::ExtentSource::AUTO) {
        VkRect2D scissor = m_node_config->getScissor();
        scissor.extent.width = extent.width;
        scissor.extent.height = extent.height;
        m_node_config->setScissor(scissor);
    }

    const std::shared_ptr<VulkanRenderPass>& render_pass_ptr = m_pipeline->getRenderPass();
    m_frame_buffer = std::make_shared<VulkanFramebuffer>();
    auto map_fn = [this](const LocalName& local_name)->const std::shared_ptr<RenderResource>&{
        return getWrittenAttachmentMap().at(local_name).resource;
    };
    m_frame_buffer->init(m_device, m_node_config->getFramebufferConfig(), render_pass_ptr, map_fn);

    for (const auto&[slot, desc_set_layout] : m_pipeline->getDescLayouts()) {
        m_descs[slot] = renderer.getDescriptorsManager()->allocateDescriptorSet(desc_set_layout->getName());
        for(const VkDescriptorSetLayoutBinding& binding : desc_set_layout->getBindings()) {
            const std::string& binding_name = desc_set_layout->getBindingName(binding.binding);
            const std::shared_ptr<GraphicsRenderNodeConfig::UpdateMetadata>& binding_metadata = m_node_config->getUpdateMetadata(binding_name);
            if(binding_metadata->resource_type == RenderResource::Type::IMAGE) {
                std::shared_ptr<VulkanImageBuffer> image_to_bind = getReadAttachedImageResource(binding_name);
                m_descs[slot]->updateDescImageInfo(
                    binding.binding,
                    image_to_bind->getImageConfig()->getSampler()->getSampler(),
                    image_to_bind->getImageBufferView(binding_metadata->image_view_type_name),
                    binding_metadata->read_image_layout
                );
            }
            else if(binding_metadata->resource_type == RenderResource::Type::BUFFER) {
                std::shared_ptr<VulkanBuffer> buffer_to_bind = getReadAttachedBufferResource(binding_name);
                m_descs[slot]->updateDescBuffer(binding.binding, buffer_to_bind->getBuffer());
            }
        }
    }
    
}

const std::shared_ptr<VulkanPipeline>& GraphicsRenderNode::getPipeline() {
    return m_pipeline;
}

VkFramebuffer GraphicsRenderNode::getFramebuffer() const {
    return m_frame_buffer->getFramebuffer();
}

const std::unordered_map<uint32_t, std::shared_ptr<VulkanDescriptor>>& GraphicsRenderNode::getDescriptors() const {
    return m_descs;
}

const std::shared_ptr<GraphicsRenderNodeConfig>& GraphicsRenderNode::getGraphicsRenderNodeConfig() const {
    return m_node_config;
}

void GraphicsRenderNode::TransitionResourcesToProperState(CommandBatch& command_buffer) {
    std::shared_ptr<RenderGraph> render_graph = m_render_graph.lock();
    const std::shared_ptr<VulkanRenderPass>& render_pass_ptr = m_pipeline->getRenderPass();

    for(const auto&[gloabal_name, att_slot] : getReadResourcesMap()){
        RenderResource::Type att_slot_res_type = att_slot.resource->getType();
        if(att_slot_res_type == RenderResource::Type::BUFFER) continue;

        size_t last_written_by_node_id = render_graph->getLastWrittenIdentity(shared_from_this(), gloabal_name);
        size_t last_read_by_node_id = render_graph->getLastReadIdentity(shared_from_this(), gloabal_name);

        if(last_read_by_node_id == RenderGraph::NO_ID && last_written_by_node_id == RenderGraph::NO_ID) {
            // std::shared_ptr<VulkanImageBuffer> attached_read_resource = render_node_ptr->getAttachedImageResource(att_slot.attached_as);
            // VkImageLayout current_image_layout = attached_read_resource->getImageConfig()->getAfterInitLayout();
            // VkImageLayout read_image_layout = render_node_ptr->getRenderNodeConfig()->getUpdateMetadata(att_slot.attached_as)->read_image_layout;
            // if(current_image_layout != read_image_layout) {
            //     attached_read_resource->changeLayout(current_image_layout, read_image_layout);
            // }
        }

        if(last_read_by_node_id != RenderGraph::NO_ID && last_read_by_node_id > last_written_by_node_id) {
            continue;
        }

        if(last_written_by_node_id != RenderGraph::NO_ID && last_written_by_node_id > last_read_by_node_id) {
            const RenderGraph::RenderNodePtr& last_written_by_node = render_graph->getRenderNodeByID(last_read_by_node_id);
            
        }
    }

    for(const auto&[gloabal_name, att_slot] : getWrittenResourcesMap()){
        RenderResource::Type att_slot_res_type = att_slot.resource->getType();
        if(att_slot_res_type == RenderResource::Type::BUFFER) continue;

        size_t last_written_by_node_id = render_graph->getLastWrittenIdentity(shared_from_this(), gloabal_name);
        size_t last_read_by_node_id = render_graph->getLastReadIdentity(shared_from_this(), gloabal_name);

        if(last_read_by_node_id == RenderGraph::NO_ID && last_written_by_node_id == RenderGraph::NO_ID) {
            std::shared_ptr<VulkanImageBuffer> attached_write_resource = getAttachedImageResource(att_slot.attached_as);
            VkImageLayout current_image_layout = attached_write_resource->getImageConfig()->getAfterInitLayout();
            
            const VkAttachmentDescription& attach_desc = m_pipeline->getRenderPass()->getRenderPassConfig()->getAttachmentDescription(att_slot.attached_as);
            VkImageLayout write_image_layout = attach_desc.initialLayout;
            if(current_image_layout != write_image_layout) {
                attached_write_resource->changeLayout(current_image_layout, write_image_layout);
                attached_write_resource->getImageConfig()->setAfterInitLayout(write_image_layout);
            }
        }

        if(last_read_by_node_id != RenderGraph::NO_ID && last_read_by_node_id > last_written_by_node_id) {
            continue;
        }

        if(last_written_by_node_id != RenderGraph::NO_ID && last_written_by_node_id > last_read_by_node_id) {
            const RenderGraph::RenderNodePtr& last_written_by_node = render_graph->getRenderNodeByID(last_read_by_node_id);
            
        }
    }
}