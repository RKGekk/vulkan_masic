#include "graphics_render_node_config.h"

#include "../api/vulkan_device.h"
#include "../api/vulkan_sampler.h"
#include "../api/vulkan_resources_manager.h"
#include "../api/vulkan_pipeline.h"
#include "../api/vulkan_pipelines_manager.h"
#include "../../window_surface.h"
#include "../api/vulkan_swapchain.h"
#include "image_buffer_config.h"
#include "format_config.h"

bool GraphicsRenderNodeConfig::init(const std::shared_ptr<VulkanDevice>& device, uint32_t fb_image_index, const std::shared_ptr<WindowSurface>& window, const SwapchainSupportDetails& swapchain_support_details, const std::shared_ptr<VulkanResourcesManager>& resources_manager, const std::shared_ptr<VulkanPipelinesManager>& pipelines_manager, const pugi::xml_node& node_data) {
    using namespace std::literals;

    m_fb_image_index = fb_image_index;
    m_name = node_data.attribute("name").as_string();
    m_framebuffer_config = resources_manager->getFramebufferConfig(node_data.child("FrameBufferName").text().as_string());
    m_pipeline = pipelines_manager->getPipeline(node_data.child("Pipeline").text().as_string());

    m_viewport = {};
    m_viewport.maxDepth = 1.0f;
    m_viewport_source = ExtentSource::AUTO;
    m_scissor = {};
    m_scissor_source = ExtentSource::AUTO;
    pugi::xml_node dynamic_states_node = node_data.child("DynamicStates");
    if(dynamic_states_node) {
        pugi::xml_node viewport_node = dynamic_states_node.child("Viewport");
        if(viewport_node) {
            pugi::xml_node x_node = viewport_node.child("x");
            if(x_node) {
                m_viewport.x = x_node.text().as_float();
            }

            pugi::xml_node y_node = viewport_node.child("y");
            if(y_node) {
                m_viewport.y = y_node.text().as_float();
            }

            pugi::xml_node width_node = viewport_node.child("width");
            if(width_node) {
                m_viewport.width = width_node.text().as_float();
            }

            pugi::xml_node height_node = viewport_node.child("height");
            if(height_node) {
                m_viewport.height = height_node.text().as_float();
            }

            pugi::xml_node minDepth_node = viewport_node.child("minDepth");
            if(minDepth_node) {
                m_viewport.minDepth = minDepth_node.text().as_float();
            }

            pugi::xml_node maxDepth_node = viewport_node.child("maxDepth");
            if(maxDepth_node) {
                m_viewport.maxDepth = maxDepth_node.text().as_float();
            }

            std::string viewport_source = viewport_node.attribute("source").as_string();
            if(viewport_source == "auto") m_viewport_source = ExtentSource::AUTO;
            else if(viewport_source == "as_swapchain") m_viewport_source = ExtentSource::AS_SWAPCHAIN;
            else if(viewport_source == "exact") m_viewport_source = ExtentSource::EXACT;
            if(m_viewport_source == ExtentSource::AS_SWAPCHAIN) {
                VkExtent2D swapchain_extent = VulkanSwapChain::chooseSwapExtent(window->GetWindow(), swapchain_support_details.capabilities);
                m_viewport.width = swapchain_extent.width;
                m_viewport.height = swapchain_extent.height;
            }
        }

        pugi::xml_node scissor_node = dynamic_states_node.child("Scissor");
        if(scissor_node) {
            pugi::xml_node x_node = scissor_node.child("OffsetX");
            if(x_node) {
                m_scissor.offset.x = x_node.text().as_float();
            }

            pugi::xml_node y_node = scissor_node.child("OffsetY");
            if(y_node) {
                m_scissor.offset.y = y_node.text().as_float();
            }

            pugi::xml_node width_node = scissor_node.child("Width");
            if(width_node) {
                m_scissor.extent.width = width_node.text().as_float();
            }

            pugi::xml_node height_node = scissor_node.child("Height");
            if(height_node) {
                m_scissor.extent.height = height_node.text().as_float();
            }

            std::string scissor_source = scissor_node.attribute("source").as_string();
            if(scissor_source == "auto") m_scissor_source = ExtentSource::AUTO;
            else if(scissor_source == "as_swapchain") m_scissor_source = ExtentSource::AS_SWAPCHAIN;
            else if(scissor_source == "exact") m_scissor_source = ExtentSource::EXACT;
            if(m_scissor_source == ExtentSource::AS_SWAPCHAIN) {
                VkExtent2D swapchain_extent = VulkanSwapChain::chooseSwapExtent(window->GetWindow(), swapchain_support_details.capabilities);
                m_scissor.extent.width = swapchain_extent.width;
                m_scissor.extent.height = swapchain_extent.height;
            }
        }
    }

    std::string draw_type_str = node_data.child("DrawType").first_child().name();
    if(draw_type_str == "Draw"s) {
        m_draw_type = DrawType::DRAW;
    }
    else if(draw_type_str == "DrawIndexed"s) {
        m_draw_type = DrawType::DRAW_INDEXED;
    }
    else if(draw_type_str == "DrawIndirect"s) {
        m_draw_type = DrawType::DRAW_INDIRECT;
    }
    else if(draw_type_str == "DrawIndirect"s) {
        m_draw_type = DrawType::DRAW_INDIRE_CTCOUNT;
    }
    else {
        m_draw_type = DrawType::DRAW_INDEXED;
    }

    m_index_count_type = IndexCountType::ALL;
    m_vertex_count_type = VertexCountType::ALL;
    m_instance_count_type = InstanceCountType::ALL;
    m_index_count = 0u;
    m_first_index = 0u;
    m_vertex_count = 0u;
    m_vertex_offset = 0;
    m_instance_count = 1u;
    m_first_instance = 0u;

    pugi::xml_node draw_type_node = node_data.child("DrawType").child("Draw");
    if(draw_type_node) {

        std::string vertex_count_type = draw_type_node.attribute("type").as_string();
        if(vertex_count_type == "all") m_vertex_count_type = VertexCountType::ALL;
        else if(vertex_count_type == "exact") m_vertex_count_type = VertexCountType::EXACT;
        
        pugi::xml_node vertex_count_node = draw_type_node.child("VertexCount");
        if(vertex_count_node) {
            m_vertex_count = vertex_count_node.text().as_uint();
        }

        pugi::xml_node instance_count_node = draw_type_node.child("InstanceCount");
        if(instance_count_node) {
            m_instance_count = instance_count_node.text().as_uint();
        }

        pugi::xml_node vertex_offset_node = draw_type_node.child("FirstVertex");
        if(vertex_offset_node) {
            m_vertex_offset = vertex_offset_node.text().as_int();
        }

        pugi::xml_node first_instance_node = draw_type_node.child("FirstInstance");
        if(first_instance_node) {
            m_first_instance = first_instance_node.text().as_uint();
        }
    }

    pugi::xml_node draw_indexed_type_node = node_data.child("DrawType").child("DrawIndexed");
    if(draw_indexed_type_node) {

        std::string index_count_type = draw_indexed_type_node.attribute("type").as_string();
        if(index_count_type == "all") m_index_count_type = IndexCountType::ALL;
        else if(index_count_type == "exact") m_index_count_type = IndexCountType::EXACT;
        
        pugi::xml_node index_count_node = draw_indexed_type_node.child("IndexCount");
        if(index_count_node) {
            m_index_count = index_count_node.text().as_uint();
        }

        pugi::xml_node instance_count_node = draw_indexed_type_node.child("InstanceCount");
        if(instance_count_node) {
            m_instance_count = instance_count_node.text().as_uint();
        }

        pugi::xml_node first_index_node = draw_indexed_type_node.child("FirstIndex");
        if(first_index_node) {
            m_first_index = first_index_node.text().as_uint();
        }

        pugi::xml_node vertex_offset_node = draw_indexed_type_node.child("VertexOffset");
        if(vertex_offset_node) {
            m_vertex_offset = vertex_offset_node.text().as_int();
        }

        pugi::xml_node first_instance_node = draw_indexed_type_node.child("FirstInstance");
        if(first_instance_node) {
            m_first_instance = first_instance_node.text().as_uint();
        }
    }

    pugi::xml_node update_metadata_node = node_data.child("DescriptorResourcesCreateAndUpdate");
    if(update_metadata_node) {
        for (pugi::xml_node layout_binding_node = update_metadata_node.first_child(); layout_binding_node; layout_binding_node = layout_binding_node.next_sibling()) {
            std::string layout_binding_name = layout_binding_node.attribute("name").as_string();
            std::shared_ptr<UpdateMetadata> metadata = std::make_shared<UpdateMetadata>();
            metadata->name = layout_binding_name;
            std::string creation_time_str = layout_binding_node.attribute("resource_creation_point").as_string(); 
            CreationPoint creation_point = creation_time_str == "RenderNodeCreationTime" ? CreationPoint::RENDER_NODE_CREATION_TIME : CreationPoint::EXTERNAL;
            metadata->creation_point = creation_point;

            if(pugi::xml_node buffer_metadata_node = layout_binding_node.child("Buffer")) {
                metadata->resource_type = RenderResource::Type::BUFFER;
                metadata->buffer_resource_type_name = buffer_metadata_node.child("BufferResourceType").text().as_string();
                metadata->update_function_name = buffer_metadata_node.child("UpdateFunctionName").text().as_string();
            }
            else if (pugi::xml_node image_metadata_node = layout_binding_node.child("Image")) {
                metadata->resource_type = RenderResource::Type::IMAGE;
                metadata->image_resource_type_name = image_metadata_node.child("ImageBufferResourceType").text().as_string();
                metadata->image_view_type_name = image_metadata_node.child("ImageViewResourceType").text().as_string();
                metadata->read_image_layout = getImageLayout(image_metadata_node.child("ReadImageLayout").text().as_string());

                if(pugi::xml_node sampler_node = image_metadata_node.child("Sampler")) {
                    std::string sampler_type_str = sampler_node.child("Type").text().as_string();
                    if(sampler_type_str == "Inline"s) metadata->sampler_type = UpdateMetadata::SamplerType::INLINE;
                    else if(sampler_type_str == "Default"s) metadata->sampler_type = UpdateMetadata::SamplerType::DEFAULT;
                    else if(sampler_type_str == "FromImageBuffer"s) metadata->sampler_type = UpdateMetadata::SamplerType::FROM_IMAGEBUFFER;
                    else metadata->sampler_type = UpdateMetadata::SamplerType::NONE;

                    if(metadata->sampler_type == UpdateMetadata::SamplerType::INLINE) {
                        VkSamplerCreateInfo sampler_info = getSamplerCreateInfo(sampler_node.child("Inline"));
                        metadata->image_sampler = std::make_shared<VulkanSampler>(device, m_name + "_inline_sampler"s);
                        metadata->image_sampler->init(sampler_info);
                    }
                }
            }
            else {
                metadata = nullptr;
            }
            m_bindings_metadata[layout_binding_name] = std::move(metadata);
        }
    }

    return true;
}


const std::string& GraphicsRenderNodeConfig::getName() const {
    return m_name;
}

const std::shared_ptr<VulkanPipeline>& GraphicsRenderNodeConfig::getPipeline() const {
    return m_pipeline;
}

VkViewport GraphicsRenderNodeConfig::getViewport() const {
    return m_viewport;
}

const VkViewport* GraphicsRenderNodeConfig::getViewportPtr() const {
    return &m_viewport;
}

GraphicsRenderNodeConfig::ExtentSource GraphicsRenderNodeConfig::getViewportSource() const {
    return m_viewport_source;
}

void GraphicsRenderNodeConfig::setViewport(VkViewport viewport) {
    m_viewport = viewport;
}

VkRect2D GraphicsRenderNodeConfig::getScissor() const {
    return m_scissor;
}

const VkRect2D* GraphicsRenderNodeConfig::getScissorPtr() const {
    return &m_scissor;
}

GraphicsRenderNodeConfig::ExtentSource GraphicsRenderNodeConfig::getScissorSource() const {
    return m_scissor_source;
}

void GraphicsRenderNodeConfig::setScissor(VkRect2D scissor) {
    m_scissor = scissor;
}

GraphicsRenderNodeConfig::IndexCountType GraphicsRenderNodeConfig::getIndexCountType() const {
    return m_index_count_type;
}
    
uint32_t GraphicsRenderNodeConfig::getIndexCount() const {
    return m_index_count;
}

GraphicsRenderNodeConfig::DrawType GraphicsRenderNodeConfig::getDrawType() const {
    return m_draw_type;
}

void GraphicsRenderNodeConfig::setDrawType(GraphicsRenderNodeConfig::DrawType draw_type) {
    m_draw_type = draw_type;
}

void GraphicsRenderNodeConfig::setIndexCount(uint32_t index_count) {
    m_index_count_type = IndexCountType::EXACT;
    m_index_count = index_count;
}

uint32_t GraphicsRenderNodeConfig::getFirstIndex() const {
    return m_first_index;
}

void GraphicsRenderNodeConfig::setFirstIndex(uint32_t first_index) {
    m_first_index = first_index;
}

int32_t GraphicsRenderNodeConfig::getVertexOffset() const {
    return m_vertex_offset;
}

GraphicsRenderNodeConfig::VertexCountType GraphicsRenderNodeConfig::getVertexCountType() const {
    return m_vertex_count_type;
}
    
uint32_t GraphicsRenderNodeConfig::getVertexCount() const {
    return m_vertex_count;
}

void GraphicsRenderNodeConfig::setVertexCount(uint32_t vtx_count) {
    m_vertex_count_type = VertexCountType::EXACT;
    m_vertex_count = vtx_count;
}

void GraphicsRenderNodeConfig::setVertexOffset(int32_t vertex_offset) {
    m_vertex_offset = vertex_offset;
}

GraphicsRenderNodeConfig::InstanceCountType GraphicsRenderNodeConfig::getInstanceCountType() const {
    return m_instance_count_type;
}

uint32_t GraphicsRenderNodeConfig::getInstanceCount() const {
    return m_instance_count;
}

void GraphicsRenderNodeConfig::setInstanceCount(uint32_t inst_ct) {
    m_instance_count_type = InstanceCountType::EXACT;
    m_instance_count = inst_ct;
}

uint32_t GraphicsRenderNodeConfig::getFirstInstance() const {
    return m_first_instance;
}

void GraphicsRenderNodeConfig::setFirstInstance(uint32_t first_instance) {
    m_first_instance = first_instance;
}

const std::shared_ptr<FramebufferConfig>& GraphicsRenderNodeConfig::getFramebufferConfig() const {
    return m_framebuffer_config;
}

const std::vector<std::shared_ptr<FramebufferConfig::FrameBufferAttachment>>& GraphicsRenderNodeConfig::getAttachmentsConfig() const {
    return m_framebuffer_config->getAttachmentsConfig();
}

const std::shared_ptr<FramebufferConfig::FrameBufferAttachment>& GraphicsRenderNodeConfig::getAttachmentData(const std::string& attached_name) const {
    return m_framebuffer_config->getAttachmentData(attached_name);
}

const std::unordered_map<std::string, std::shared_ptr<GraphicsRenderNodeConfig::UpdateMetadata>>& GraphicsRenderNodeConfig::getBindingsMetadata() const {
    return m_bindings_metadata;
}

std::shared_ptr<GraphicsRenderNodeConfig> GraphicsRenderNodeConfig::makeInstance(std::string name) const {
    std::shared_ptr<GraphicsRenderNodeConfig> instance = std::make_shared<GraphicsRenderNodeConfig>();
    *instance = *this;
    instance->m_name = std::move(name);

    return instance;
}

const std::shared_ptr<GraphicsRenderNodeConfig::UpdateMetadata>& GraphicsRenderNodeConfig::getUpdateMetadata(const std::string& binding_name) {
    return m_bindings_metadata.at(binding_name);
}