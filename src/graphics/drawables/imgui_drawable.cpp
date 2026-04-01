#include "imgui_drawable.h"

#include "../vulkan_renderer.h"
#include "../pod/image_buffer_config.h"
#include "../pod/format_config.h"
#include "../pod/render_node.h"
#include "../pod/render_node_config.h"
#include "../pod/pipeline_config.h"
#include "../pod/basic_uniform.h"
#include "../api/vulkan_pipelines_manager.h"
#include "../api/vulkan_resources_manager.h"
#include "../api/vulkan_descriptors_manager.h"
#include "../api/vulkan_image_buffer.h"
#include "../api/vulkan_swapchain.h"
#include "../../application.h"

#include <pugixml.hpp>

#include <filesystem>

std::shared_ptr<VulkanImageBuffer> makeFontTexture(std::shared_ptr<VulkanDevice> device, const char* TTF_font_file_name, float fontSizePixels) {
    ImGuiIO& io = ImGui::GetIO();

    ImFontConfig cfg = ImFontConfig();
    cfg.FontDataOwnedByAtlas = false;
    cfg.RasterizerMultiply = 1.0f;
    cfg.SizePixels = ceilf(fontSizePixels);
    cfg.PixelSnapH = true;
    cfg.OversampleH = 1;
    cfg.OversampleV = 1;
    ImFont* font = nullptr;

    if (TTF_font_file_name) {
        font = io.Fonts->AddFontFromFileTTF(TTF_font_file_name, cfg.SizePixels, &cfg);
    }
    else {
        font = io.Fonts->AddFontDefault(&cfg);
    }

    io.Fonts->Flags |= ImFontAtlasFlags_NoPowerOfTwoHeight;

    // init fonts
    unsigned char* pixels;
    int width;
    int height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

    std::shared_ptr<VulkanImageBuffer> font_texture = Application::GetRenderer().getResourcesManager()->create_image(pixels, {(uint32_t)width, (uint32_t)height}, TTF_font_file_name, "imgui_font_resource");

    io.Fonts->TexID = 0u;
    io.FontDefault = font;

    return font_texture;
}

std::shared_ptr<RenderNodeConfig> getImguiRenderNodeConfig(const std::shared_ptr<VulkanDevice>& device) {
    using namespace std::literals;

    pugi::xml_document xml_doc;
	pugi::xml_parse_result parse_res = xml_doc.load_file("graphics_pipelines.xml");
	if (!parse_res) { return nullptr;	}

	pugi::xml_node root_node = xml_doc.root();
	if (!root_node) { return nullptr; }
	root_node = root_node.child("RenderGraph");

    pugi::xml_node render_nodes_node = root_node.child("RenderNodes");
	if (!render_nodes_node) return nullptr;

    for (pugi::xml_node render_node = render_nodes_node.first_child(); render_node; render_node = render_node.next_sibling()) {
        std::string node_name = render_node.attribute("name").as_string();
        if(node_name == "imgui_renderer"s) {
            std::shared_ptr<RenderNodeConfig> render_node_config = std::make_shared<RenderNodeConfig>();
            render_node_config->init(device, Application::GetRenderer().getResourcesManager(), render_node);
            return render_node_config;
        }
    }

    return nullptr;
}

bool ImGUIDrawable::init(std::shared_ptr<VulkanDevice> device, int max_frames) {
    using namespace std::literals;

    m_device = std::move(device);
    m_max_frames = max_frames;

    static const std::string TTF_font_file_name = std::filesystem::current_path().append("fonts").append("OpenSans-Light.ttf").string();
    static const float font_size_pixels = 15.0f;

    m_pImgui_ctx = ImGui::CreateContext();
    ImGui::SetCurrentContext(m_pImgui_ctx);

    ImGuiIO& io = ImGui::GetIO();
    io.BackendRendererName = "imgui-lvk";
    io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;
    io.FontAllowUserScaling = true;
    io.FontGlobalScale = 1.0f;
    io.KeyRepeatDelay = 1.5f;
    io.KeyRepeatRate = 0.2f;
    ImGui::StyleColorsClassic();

    std::shared_ptr<VulkanImageBuffer> font_texture = makeFontTexture(m_device, TTF_font_file_name.c_str(), font_size_pixels);
    
    std::shared_ptr<RenderNodeConfig> render_node_config = getImguiRenderNodeConfig(m_device);

    const std::vector<std::shared_ptr<VulkanImageBuffer>>& swapchain_images = Application::GetRenderer().getSwapchain()->getSwapchainImages();

    for(size_t frame = 0u; frame < max_frames; ++frame) {
        std::shared_ptr<Renderable> renderable = std::make_shared<Renderable>();

        renderable->frame = frame;
        renderable->font_texture = font_texture;

        renderable->imgui_vtx.resize(max_frames);
        renderable->imgui_idx.resize(max_frames);
        renderable->vertex_buffer = Application::GetRenderer().getResourcesManager()->create_buffer(nullptr, 0, "imgui_vertex_buffer_frame_"s + std::to_string(frame), "imgui_vertex_resource");
        renderable->index_buffer = Application::GetRenderer().getResourcesManager()->create_buffer(nullptr, 0, "imgui_index_buffer_frame_"s + std::to_string(frame), "imgui_index_resource");
        renderable->uniform_buffer = Application::GetRenderer().getResourcesManager()->create_buffer(nullptr, 0, "imgui_uniform_buffer_frame_"s + std::to_string(frame), "imgui_uniform_resource");

        renderable->render_node = std::make_shared<RenderNode>();
        renderable->render_node->init(m_device, render_node_config);

        std::shared_ptr<VulkanShader> vertex_shader = renderable->render_node->getPipeline()->getShader(VK_SHADER_STAGE_VERTEX_BIT);
        std::shared_ptr<DescSetLayout> desc_set_layout = Application::GetRenderer().getDescriptorsManager()->getDescSetLayout(vertex_shader->getShaderSignature()->getDescSetNames().at(0));

        renderable->render_node->addReadDependency(renderable->vertex_buffer, vertex_shader->getShaderSignature()->getVertexFormat().getVertexBufferBindingName());
        renderable->render_node->addReadDependency(renderable->index_buffer, vertex_shader->getShaderSignature()->getVertexFormat().getIndexBufferBindingName());
        renderable->render_node->addReadDependency(renderable->uniform_buffer, desc_set_layout->getBindingName(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER));
        renderable->render_node->addReadDependency(renderable->font_texture, desc_set_layout->getBindingName(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER));
        renderable->render_node->addWriteDependency(swapchain_images[frame], "resolve_attachment");
        renderable->render_node->addWriteDependency(Application::GetRenderer().getOutColorImage(frame), "color_attachment");
        renderable->render_node->addWriteDependency(Application::GetRenderer().getOutDepthImage(frame), "depth_attachment");
        renderable->render_node->finishRenderNode();

        m_renderables.push_back(renderable);
        Application::GetRenderer().addRenderNode(renderable->render_node, frame);
        
    }
    

    return true;
}

void ImGUIDrawable::reset() {
    
}

void ImGUIDrawable::destroy() {
    
}

void ImGUIDrawable::update(const GameTimerDelta& delta, uint32_t image_index) {
    std::shared_ptr<Renderable>& renderable = m_renderables.at(image_index);

    ImDrawData* dd = ImGui::GetDrawData();
    const float L = dd->DisplayPos.x;
    const float R = dd->DisplayPos.x + dd->DisplaySize.x;
    const float T = dd->DisplayPos.y;
    const float B = dd->DisplayPos.y + dd->DisplaySize.y;
    std::shared_ptr<VulkanShader> vertex_shader = renderable->render_node->getPipeline()->getShader(VK_SHADER_STAGE_VERTEX_BIT);
    size_t index_count = renderable->index_buffer->getNotAlignedSize() / vertex_shader->getShaderSignature()->getVertexFormat().getIndexTypeBytesCount();
    size_t vertex_count = renderable->vertex_buffer->getNotAlignedSize() / vertex_shader->getShaderSignature()->getVertexFormat().getVertexSize();

    if(vertex_count < dd->TotalVtxCount) {
        renderable->imgui_vtx.resize(dd->TotalVtxCount);
    }

    if(index_count < dd->TotalIdxCount) {
        renderable->imgui_idx.resize(dd->TotalIdxCount);
    }

    int vertex_sz = 0;
    int index_sz = 0;
    uint32_t vtxOffset = 0;
    uint32_t idxOffset = 0;
    ImDrawVert* vtx = (ImDrawVert*)renderable->imgui_vtx.data();
    uint16_t* idx = (uint16_t*)renderable->imgui_idx.data();
    for (const ImDrawList* cmdList : dd->CmdLists) {
        size_t vtx_sz = cmdList->VtxBuffer.Size * sizeof(ImDrawVert);
        size_t idx_sz = cmdList->IdxBuffer.Size * sizeof(ImDrawIdx);

        memcpy(vtx, cmdList->VtxBuffer.Data, vtx_sz);
        memcpy(idx, cmdList->IdxBuffer.Data, idx_sz);
        vtx += cmdList->VtxBuffer.Size;
        idx += cmdList->IdxBuffer.Size;

        vertex_sz += vtx_sz;
        index_sz += idx_sz;
    }

    renderable->vertex_buffer->update(renderable->imgui_vtx.data(), vertex_sz);
    renderable->index_buffer->update(renderable->imgui_idx.data(), index_sz);

    ImGuiUniformBufferObject bindData;
    bindData.LRTB = {L, R, T, B};
    renderable->uniform_buffer->update(&bindData, sizeof(ImGuiUniformBufferObject));

    //const std::vector<std::shared_ptr<VulkanImageBuffer>>& swapchain_images = Application::GetRenderer().getSwapchain()->getSwapchainImages();
    //renderable->render_node->changeWriteDependency(swapchain_images[image_index], "resolve_attachment");
}

int ImGUIDrawable::order() {
    return 999;
}

void ImGUIDrawable::beginFrame() {
    ImGui::SetCurrentContext(m_pImgui_ctx);
    ImGuiIO& io = ImGui::GetIO();
    VkExtent2D extent = Application::GetRenderer().getSwapchain()->getFormatConfig()->getExtent2D();
    io.DisplaySize = ImVec2(extent.width, extent.height);
    io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
    io.IniFilename = nullptr;

    ImGui::NewFrame();
}

void ImGUIDrawable::endFrame() {
    ImGui::SetCurrentContext(m_pImgui_ctx);
    ImGui::EndFrame();
    ImGui::Render();
}