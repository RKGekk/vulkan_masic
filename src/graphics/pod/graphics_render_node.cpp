#include "graphics_render_node.h"

#include "graphics_render_node_config.h"
#include "render_graph.h"
#include "render_node.h"
#include "image_buffer_config.h"
#include "format_config.h"
#include "render_pass_config.h"
#include "buffer_config.h"
#include "../../application.h"
#include "../vulkan_renderer.h"
#include "../api/vulkan_pipelines_manager.h"
#include "../api/vulkan_render_pass.h"
#include "../api/vulkan_descriptors_manager.h"
#include "../api/vulkan_framebuffer.h"
#include "../api/vulkan_resources_manager.h"
#include "../../scene/skeleton_manager.h"

#include <utility>

const GraphicsRenderNode::BindFlags GraphicsRenderNode::ALL_BIND_FLAGS = GraphicsRenderNode::BindFlags::Pipeline | GraphicsRenderNode::BindFlags::PushConstants | GraphicsRenderNode::BindFlags::Viewport | GraphicsRenderNode::BindFlags::Scissor | GraphicsRenderNode::BindFlags::DescriptorSets | GraphicsRenderNode::BindFlags::VertexIndexInstanceBuffers | GraphicsRenderNode::BindFlags::Draw;

bool GraphicsRenderNode::init(std::string render_node_name, unsigned frame_index, std::shared_ptr<VulkanDevice> device, VkCommandBuffer secondary_command_buffer, const std::string& node_config_name, bool instance_config, std::weak_ptr<RenderGraph> render_graph) {
    m_render_node_name = std::move(render_node_name);
    m_frame_index = frame_index;
    m_device = std::move(device);
    m_secondary_command_buffer = secondary_command_buffer;
    m_render_graph = std::move(render_graph);
    m_node_config = instance_config ? m_render_graph.lock()->makeGraphicsRenderNodeCfgInstance(node_config_name, std::to_string(rand())) : m_render_graph.lock()->getGraphicsRenderNodeConfig(node_config_name);
    m_pipeline = m_node_config->getPipeline();
    setExecutionBypass(false);
    setExecutionOrder(0u);

    VulkanRenderer& renderer = Application::GetRenderer();
    std::shared_ptr<VulkanResourcesManager>& resources_manager = renderer.getResourcesManager();

    for(const auto&[desc_slot, desc_set_layout] : m_pipeline->getDescLayouts()) {
        for (const auto&[desc_layout_bind_name, bind_num] : desc_set_layout->getBindingMap()) {

            const VkDescriptorSetLayoutBinding& vk_layout_binding = desc_set_layout->getBinding(bind_num);
            const std::shared_ptr<GraphicsRenderNodeConfig::UpdateMetadata>& update_metadata = m_node_config->getBindingsMetadata().at(desc_layout_bind_name);

            if(update_metadata->resource_type == RenderResource::Type::IMAGE) {
                continue;
            }

            if(update_metadata->resource_type == RenderResource::Type::PUSH_CONSTANT) {
                continue;
            }

            const std::shared_ptr<BufferConfig>& buffer_config = resources_manager->getBufferConfigTemplate(update_metadata->buffer_resource_type_name);
            bool can_instance = update_metadata->creation_point == GraphicsRenderNodeConfig::CreationPoint::RENDER_NODE_CREATION_TIME && buffer_config->getBufferInfo().size > 0u;

            if(vk_layout_binding.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER && can_instance) {
                std::string ubo_buffer_name = m_render_node_name + "/uniform/"s + desc_layout_bind_name + "/frame_"s + std::to_string(m_frame_index);
                std::shared_ptr<VulkanBuffer> ubo = resources_manager->create_buffer(nullptr, 0, std::move(ubo_buffer_name), update_metadata->buffer_resource_type_name);
                ubo->AddUpdateMetadataFn(
                    [this, &desc_layout_bind_name]
                    (const std::shared_ptr<RenderResource>& bo){
                        changeReadDependency(bo, desc_layout_bind_name);
                        updateDescriptor(desc_layout_bind_name);
                    }
                );
                addReadDependency(std::move(ubo), desc_layout_bind_name);
            }
            else if(vk_layout_binding.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER && update_metadata->creation_point == GraphicsRenderNodeConfig::CreationPoint::EXTERNAL) {
                std::string ubo_global_name = desc_layout_bind_name + std::to_string(m_frame_index);
                if(resources_manager->hasResource(ubo_global_name)) {
                    const std::shared_ptr<VulkanBuffer>& ubo = resources_manager->getBufferResource(ubo_global_name);
                    ubo->AddUpdateMetadataFn(
                        [this, &desc_layout_bind_name]
                        (const std::shared_ptr<RenderResource>& bo){
                            changeReadDependency(bo, desc_layout_bind_name);
                            updateDescriptor(desc_layout_bind_name);
                        }
                    );
                    addReadDependency(ubo, desc_layout_bind_name);
                }
            }
            else if(vk_layout_binding.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER && can_instance) {
                std::string ssbo_buffer_name = m_render_node_name + "/storage/"s + desc_layout_bind_name + "/frame_"s + std::to_string(m_frame_index);
                std::shared_ptr<VulkanBuffer> ssbo = resources_manager->create_buffer(nullptr, 0, ssbo_buffer_name, update_metadata->buffer_resource_type_name);
                ssbo->AddUpdateMetadataFn(
                    [this, &desc_layout_bind_name]
                    (const std::shared_ptr<RenderResource>& bo){
                        changeReadDependency(bo, desc_layout_bind_name);
                        updateDescriptor(desc_layout_bind_name);
                    }
                );
                addReadDependency(std::move(ssbo), desc_layout_bind_name);
            }
            else if(vk_layout_binding.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER && update_metadata->creation_point == GraphicsRenderNodeConfig::CreationPoint::EXTERNAL) {
                std::string ssbo_global_name = desc_layout_bind_name + std::to_string(m_frame_index);
                if(resources_manager->hasResource(ssbo_global_name)) {
                    std::shared_ptr<VulkanBuffer> ssbo = resources_manager->getBufferResource(ssbo_global_name);
                    ssbo->AddUpdateMetadataFn(
                        [this, &desc_layout_bind_name]
                        (const std::shared_ptr<RenderResource>& bo){
                            changeReadDependency(bo, desc_layout_bind_name);
                            updateDescriptor(desc_layout_bind_name);
                        }
                    );
                    addReadDependency(std::move(ssbo), desc_layout_bind_name);
                }
            }
        }
    }

    return true;
}

void GraphicsRenderNode::destroy() {
    m_frame_buffer->destroy();
}

void GraphicsRenderNode::render(VkCommandBuffer secondary_command_buffer, uint32_t bind_flags) {
    BindFlags enum_bind = static_cast<BindFlags>(bind_flags);

    if(getExecutionBypass()) return;
    if(!m_dirty_read_dependency) return;

    if(m_node_config->getDrawType() == GraphicsRenderNodeConfig::DrawType::DRAW && m_node_config->getVertexCountType() == GraphicsRenderNodeConfig::VertexCountType::EXACT && m_node_config->getVertexCount() == 0u) return;
    if(m_node_config->getDrawType() == GraphicsRenderNodeConfig::DrawType::DRAW_INDEXED && m_node_config->getIndexCountType() == GraphicsRenderNodeConfig::IndexCountType::EXACT && m_node_config->getIndexCount() == 0u) return;

    for(const auto&[desc_layout_binding_name, metadata] : m_node_config->getBindingsMetadata()) {
        if(metadata->creation_point == GraphicsRenderNodeConfig::CreationPoint::EXTERNAL) continue;
        
        const std::string& update_fn_name = metadata->update_function_name;
        std::shared_ptr<VulkanBuffer> uniform_buffer = getAttachedBufferResource(desc_layout_binding_name);
        getUpdateFunction(update_fn_name)(uniform_buffer, desc_layout_binding_name);
    }
    
    if(has_flag(enum_bind, BindFlags::Pipeline)) {
        vkCmdBindPipeline(secondary_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline->getPipeline());
    }

    if(has_flag(enum_bind, BindFlags::PushConstants)) {
        m_pipeline->build_push_constants();
        m_pipeline->attach_push_constants(secondary_command_buffer);
    }

    if(has_flag(enum_bind, BindFlags::Scissor)) {
        VkRect2D scissor = m_node_config->getScissor();
        vkCmdSetScissor(secondary_command_buffer, 0u, 1u, &scissor);
    }

    if(has_flag(enum_bind, BindFlags::Viewport)) {
        VkViewport view_port = m_node_config->getViewport();
        vkCmdSetViewport(secondary_command_buffer, 0u, 1u, &view_port);
    }
        
    if(has_flag(enum_bind, BindFlags::DescriptorSets)) {
        for(const auto&[slot, desc] : getDescriptors()) {
            vkCmdBindDescriptorSets(
                secondary_command_buffer, // commandBuffer
                VK_PIPELINE_BIND_POINT_GRAPHICS, // pipelineBindPoint
                m_pipeline->getPipelineLayout(), // pipeline layout
                slot, // first set
                1, // descriptor set count
                desc->getDescriptorSetPtr(), // descriptor sets pointer
                0, // dynamic offset count
                nullptr // dynamic offsets pointer
            );
        }
    }

    if(has_flag(enum_bind, BindFlags::VertexIndexInstanceBuffers) || has_flag(enum_bind, BindFlags::Draw)) {
        const std::shared_ptr<ShaderSignature>& vertex_shader_signature = m_pipeline->getShader(VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT)->getShaderSignature();
        const std::vector<VertexFormat>& vertex_formats_array = vertex_shader_signature->getInputAttributes();

        m_vertex_buffers.clear();
        m_vertex_buffers_offsets.clear();
        m_first_binding = vertex_formats_array.front().getBindingNum();
        m_vertex_count = 0u;
        m_instance_count = 1u;
        for (const VertexFormat& vf : vertex_formats_array) {
            if(vf.getBindingNum() < m_first_binding) {
                m_first_binding = vf.getBindingNum();
            }
            const std::string& vertex_buffer_name = vf.getVertexBufferBindingName();
            std::shared_ptr<VulkanBuffer> vertex_buffer = getReadAttachedBufferResource(vertex_buffer_name);
            m_vertex_buffers.push_back(vertex_buffer->getBuffer());
            m_vertex_buffers_offsets.push_back(vf.getVertexBufferOffset());

            if(vf.getInputRate() == VkVertexInputRate::VK_VERTEX_INPUT_RATE_VERTEX) {
                m_vertex_count = vertex_buffer->getNotAlignedSize() / vf.getVertexSize();
            }
            else if(vf.getInputRate() == VkVertexInputRate::VK_VERTEX_INPUT_RATE_INSTANCE) {
                m_instance_count = vertex_buffer->getNotAlignedSize() / vf.getVertexSize();
            }
        }

        vkCmdBindVertexBuffers(
            secondary_command_buffer,               // commandBuffer
            m_first_binding,                                // firstBinding
            static_cast<uint32_t>(m_vertex_buffers.size()), // bindingCount
            m_vertex_buffers.data(),                        // buffers pointer
            m_vertex_buffers_offsets.data()                 // offsets pointer
        );
        
        if(m_node_config->getDrawType() == GraphicsRenderNodeConfig::DrawType::DRAW) {
            if(m_node_config->getIndexCountType() == GraphicsRenderNodeConfig::IndexCountType::ALL) {
                vkCmdDraw(
                    secondary_command_buffer,   // commandBuffer
                    m_vertex_count,                     // vertexCount
                    m_instance_count,                   // instanceCount
                    0u,                                 // firstVertex
                    0u                                  // firstInstance
                );
            }
            else {
                vkCmdDraw(
                    secondary_command_buffer,   // commandBuffer
                    m_node_config->getVertexCount(),    // vertexCount
                    m_node_config->getInstanceCount(),  // instanceCount
                    m_node_config->getFirstIndex(),     // firstVertex
                    m_node_config->getFirstInstance()   // firstInstance
                );
            }
        }
        else if(m_node_config->getDrawType() == GraphicsRenderNodeConfig::DrawType::DRAW_INDEXED) {
            const std::string& index_buffer_name = vertex_shader_signature->getIndexBufferBindingName();
            std::shared_ptr<VulkanBuffer> index_buffer = getReadAttachedBufferResource(index_buffer_name);
            vkCmdBindIndexBuffer(
                secondary_command_buffer,       // commandBuffer
                index_buffer->getBuffer(),              // buffer
                vertex_shader_signature->getIndexBufferOffset(), // offset
                vertex_shader_signature->getIndexType()          // indexType
            );

            if(m_node_config->getIndexCountType() == GraphicsRenderNodeConfig::IndexCountType::ALL) {
                uint32_t index_count = index_buffer->getNotAlignedSize() / vertex_shader_signature->getIndexTypeBytesCount();
                vkCmdDrawIndexed(
                    secondary_command_buffer,   // commandBuffer
                    index_count,                        // indexCount
                    m_instance_count,                   // instanceCount
                    0u,                                 // firstIndex
                    0u,                                 // vertexOffset
                    0u                                  // firstInstance
                );
            }
            else {
                uint32_t index_count = m_node_config->getIndexCount();
                vkCmdDrawIndexed(
                    secondary_command_buffer,   // commandBuffer
                    index_count,                        // indexCount
                    m_node_config->getInstanceCount(),  // instanceCount
                    m_node_config->getFirstIndex(),     // firstIndex
                    m_node_config->getVertexOffset(),   // vertexOffset
                    m_node_config->getFirstInstance()   // firstInstance
                );
            }
        }
    }

    m_dirty_read_dependency = false;
    m_dirty_write_dependency = false;
}

void GraphicsRenderNode::initFramebuffer() {
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

    if(m_frame_buffer) {
        m_frame_buffer->destroy();
    }
    // else {
    //     m_frame_buffer = std::make_shared<VulkanFramebuffer>();
    // }

    auto map_fn = [this](const LocalName& local_name)->const std::shared_ptr<RenderResource>&{
        return getWrittenAttachmentMap().at(local_name).resource;
    };
    const std::shared_ptr<VulkanRenderPass>& render_pass_ptr = m_pipeline->getRenderPass();
    std::string fb_name = m_node_config->getFramebufferConfig()->getName() + render_pass_ptr->getRenderPassConfig()->getName();
    m_frame_buffer = Application::GetRenderer().getResourcesManager()->create_frame_buffer(map_fn, render_pass_ptr, fb_name, m_node_config->getFramebufferConfig()->getName());
    //m_frame_buffer->init(m_device, m_node_config->getFramebufferConfig(), render_pass_ptr, map_fn);
}

void GraphicsRenderNode::updateDescriptors() {
    VulkanRenderer& renderer = Application::GetRenderer();
    for (const auto&[slot, desc_set_layout] : m_pipeline->getDescLayouts()) {
        for(const VkDescriptorSetLayoutBinding& binding : desc_set_layout->getBindings()) {
            updateDescriptor(slot, binding.binding);
        }
    }
}

void GraphicsRenderNode::updateDescriptor(const std::string& desc_binding_name) {
    if(!m_pipeline->hasDescBinding(desc_binding_name)) return;

    const std::shared_ptr<DescSetLayout>& desc_set_layout = m_pipeline->getDescLayout(desc_binding_name);
    VulkanPipeline::DescBindingSlot slot = m_pipeline->getDescBindingSlot(desc_binding_name);
    VkDescriptorSetLayoutBinding desc_bind = desc_set_layout->getBinding(desc_binding_name);
    updateDescriptor(slot, desc_bind.binding);
}

void GraphicsRenderNode::updateDescriptor(uint32_t desc_slot, uint32_t desc_binding_num) {
    const std::shared_ptr<DescSetLayout>& desc_set_layout = m_pipeline->getDescLayout(desc_slot);
    std::shared_ptr<VulkanDescriptor> desc_ptr = getDescriptor(desc_slot);
    if(!desc_ptr) {
        VulkanRenderer& renderer = Application::GetRenderer();        
        desc_ptr = renderer.getDescriptorsManager()->allocateDescriptorSet(desc_set_layout->getName());
        setDescriptor(desc_slot, desc_ptr);
    }

    const std::string& binding_name = desc_set_layout->getBindingName(desc_binding_num);
    if(!isReadAttached(binding_name)) return;
            
    const std::shared_ptr<GraphicsRenderNodeConfig::UpdateMetadata>& binding_metadata = m_node_config->getUpdateMetadata(binding_name);
    if(binding_metadata->resource_type == RenderResource::Type::IMAGE) {
        std::shared_ptr<VulkanImageBuffer> image_to_bind = getReadAttachedImageResource(binding_name);
        desc_ptr->updateDescImageInfo(
            desc_binding_num,
            image_to_bind->getImageConfig()->getSampler()->getSampler(),
            image_to_bind->getImageBufferView(binding_metadata->image_view_type_name),
            binding_metadata->read_image_layout
        );
    }
    else if(binding_metadata->resource_type == RenderResource::Type::BUFFER) {
        std::shared_ptr<VulkanBuffer> buffer_to_bind = getReadAttachedBufferResource(binding_name);
        desc_ptr->updateDescBuffer(desc_binding_num, buffer_to_bind->getBuffer());
    }
}

void GraphicsRenderNode::finishRenderNode() {
    initFramebuffer();
    updateDescriptors();
}

RenderNode::RenderNodeType GraphicsRenderNode::getRenderNodeType() const {
    return RenderNode::RenderNodeType::Graphics;
}

const std::shared_ptr<VulkanPipeline>& GraphicsRenderNode::getPipeline() {
    return m_pipeline;
}

VkFramebuffer GraphicsRenderNode::getVkFramebuffer() const {
    return m_frame_buffer->getFramebuffer();
}

const std::shared_ptr<VulkanFramebuffer>& GraphicsRenderNode::getFB() const {
    return m_frame_buffer;
}

std::shared_ptr<GraphicsRenderNodeConfig>& GraphicsRenderNode::getGraphicsRenderNodeConfig() {
    return m_node_config;
}

void GraphicsRenderNode::TransitionResourcesToProperState(VkCommandBuffer primary_command_buffer) {
    std::shared_ptr<RenderGraph> render_graph = m_render_graph.lock();
    const std::shared_ptr<VulkanRenderPass>& render_pass_ptr = m_pipeline->getRenderPass();

    for(const auto&[gloabal_name, att_slot] : getWrittenResourcesMap()){
        RenderResource::Type att_slot_res_type = att_slot.resource->getType();
        if(att_slot_res_type == RenderResource::Type::BUFFER) continue;

        bool resource_only_init = render_graph->isWrittenInGraph(gloabal_name);
        if(resource_only_init) {
            std::shared_ptr<VulkanImageBuffer> attached_write_resource = getAttachedImageResource(att_slot.attached_as);
            VkImageLayout current_image_layout = attached_write_resource->getImageConfig()->getAfterInitLayout();
            
            const VkAttachmentDescription& attach_desc = m_pipeline->getRenderPass()->getRenderPassConfig()->getAttachmentDescription(att_slot.attached_as);
            VkImageLayout write_image_layout = attach_desc.initialLayout;
            if(current_image_layout != write_image_layout) {
                attached_write_resource->changeLayout(current_image_layout, write_image_layout);
                attached_write_resource->getImageConfig()->setAfterInitLayout(write_image_layout);
            }
            continue;
        }

        // size_t my_pos = render_graph->getTopologicalIdentity(shared_from_this());
        // size_t last_written_by_node_id = render_graph->getLastWrittenIdentity(shared_from_this(), gloabal_name);
        // size_t last_read_by_node_id = render_graph->getLastReadIdentity(shared_from_this(), gloabal_name);

        // if(last_read_by_node_id == RenderGraph::NO_ID && last_written_by_node_id == RenderGraph::NO_ID) {
        //     std::shared_ptr<VulkanImageBuffer> attached_write_resource = getAttachedImageResource(att_slot.attached_as);
        //     VkImageLayout current_image_layout = attached_write_resource->getImageConfig()->getAfterInitLayout();
            
        //     const VkAttachmentDescription& attach_desc = m_pipeline->getRenderPass()->getRenderPassConfig()->getAttachmentDescription(att_slot.attached_as);
        //     VkImageLayout write_image_layout = attach_desc.initialLayout;
        //     if(current_image_layout != write_image_layout) {
        //         attached_write_resource->changeLayout(current_image_layout, write_image_layout);
        //         attached_write_resource->getImageConfig()->setAfterInitLayout(write_image_layout);
        //     }
        // }
    }
}