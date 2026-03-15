#include "imgui_drawable.h"

#include "../vulkan_renderer.h"
#include "../pod/image_buffer_config.h"
#include "../pod/format_config.h"
#include "../pod/render_node.h"
#include "../pod/render_node_config.h"
#include "../pod/pipeline_config.h"
#include "../api/vulkan_pipelines_manager.h"
#include "../api/vulkan_image_buffer.h"
#include "../../application.h"

#include <pugixml.hpp>

#include <filesystem>

std::shared_ptr<VulkanImageBuffer> makeFontTexture(std::shared_ptr<VulkanDevice> device, std::shared_ptr<Managers>& managers, const char* TTF_font_file_name, float fontSizePixels) {
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
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

    std::shared_ptr<VulkanImageBuffer> font_texture = managers->resources_manager->create_image(TTF_font_file_name);

    io.Fonts->TexID = 0u;
    io.FontDefault = font;

    return font_texture;
}

std::shared_ptr<RenderNodeConfig> getImguiRenderNodeConfig(const std::shared_ptr<VulkanDevice>& device) {
    std::shared_ptr<RenderNodeConfig> render_node_config = std::make_shared<RenderNodeConfig>();

    pugi::xml_document xml_doc;
	pugi::xml_parse_result parse_res = xml_doc.load_file("graphics_pipelines.xml");
	if (!parse_res) { return nullptr;	}

	pugi::xml_node root_node = xml_doc.root();
	if (!root_node) { return nullptr; }
	root_node = root_node.child("RenderGraph");

    pugi::xml_node render_nodes_node = root_node.child("RenderNodes");
	if (!render_nodes_node) return nullptr;

    render_node_config->init(device, render_nodes_node.child("imgui_renderer"));

    return render_node_config;
}

bool ImGUIDrawable::init(std::shared_ptr<VulkanDevice> device, std::shared_ptr<Managers>& managers, int max_frames) {
    using namespace std::literals;

    m_device = std::move(device);
    m_max_frames = max_frames;

    m_imgui_vtx.resize(max_frames);
    m_imgui_idx.resize(max_frames);

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

    m_font_texture = makeFontTexture(m_device, managers, TTF_font_file_name.c_str(), font_size_pixels);

    std::shared_ptr<RenderNodeConfig> render_node_config = getImguiRenderNodeConfig(device);

    const std::vector<std::shared_ptr<VulkanImageBuffer>>& swapchain_images = Application::GetRenderer().getSwapchain()->getSwapchainImages();
    std::vector<std::shared_ptr<VulkanImageBuffer>>& color_images = Application::GetRenderer().getOutColorImages();
    std::vector<std::shared_ptr<VulkanImageBuffer>>& depth_images = Application::GetRenderer().getOutDepthImages();

    m_render_nodes.resize(max_frames);
    m_vertex_buffers.resize(max_frames);
    m_uniform_buffers.resize(max_frames);
    for(size_t i = 0u; i < max_frames; ++i) {

        m_vertex_buffers[i] = managers->resources_manager->create_buffer(nullptr, 0, "imgui_vertex_resource");
        m_index_buffers[i] = managers->resources_manager->create_buffer(nullptr, 0, "imgui_index_resource");
        m_uniform_buffers[i] = managers->resources_manager->create_buffer(nullptr, 0, "imgui_uniform_resource");

        m_render_nodes[i] = std::make_shared<RenderNode>();
        m_render_nodes[i]->init(device, render_node_config);

        std::shared_ptr<VulkanShader> vertex_shader = m_render_nodes[i]->getPipeline()->getShader(VK_SHADER_STAGE_VERTEX_BIT);
        std::shared_ptr<DescSetLayout> desc_set_layout = managers->descriptors_manager->getDescSetLayout(vertex_shader->getShaderSignature()->getDescSetNames()[0]);

        m_render_nodes[i]->addReadDependency(m_vertex_buffers[i], vertex_shader->getShaderSignature()->getVertexFormat().getVertexBufferBindingName());
        m_render_nodes[i]->addReadDependency(m_index_buffers[i], vertex_shader->getShaderSignature()->getVertexFormat().getIndexBufferBindingName());
        m_render_nodes[i]->addReadDependency(m_uniform_buffers[i], desc_set_layout->getBindingName(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER));
        m_render_nodes[i]->addReadDependency(m_font_texture, desc_set_layout->getBindingName(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER));
        m_render_nodes[i]->addWriteDependency(swapchain_images[i], "resolve_attachment");
        m_render_nodes[i]->addWriteDependency(color_images[i], "color_attachment");
        m_render_nodes[i]->addWriteDependency(depth_images[i], "depth_attachment");

        m_render_nodes[i]->finishRenderNode();
        Application::GetRenderer().addRenderNode(m_render_nodes[i]);
    }

    return true;
}

void ImGUIDrawable::reset() {
    
}

void ImGUIDrawable::destroy() {
    
}

void ImGUIDrawable::update(const GameTimerDelta& delta, uint32_t image_index) {
    float angle = delta.fGetTotalSeconds() * glm::radians(90.f);
    glm::vec3 rotation_axis = glm::vec3(0.0f, 0.0f, 1.0f);
    ImDrawData* dd = ImGui::GetDrawData();
    std::shared_ptr<VulkanShader> vertex_shader = m_render_nodes[image_index]->getPipeline()->getShader(VK_SHADER_STAGE_VERTEX_BIT);
    size_t vertex_count = m_index_buffers[image_index]->getSize() / vertex_shader->getShaderSignature()->getVertexFormat().getIndexTypeBytesCount();

    if(vertex_count < dd->TotalVtxCount) {
        m_imgui_vtx[image_index].resize(dd->TotalVtxCount);
        m_imgui_idx[image_index].resize(dd->TotalIdxCount);
    }

    int vertex_sz = 0;
    int index_sz = 0;
    uint32_t vtxOffset = 0;
    uint32_t idxOffset = 0;
    ImDrawVert* vtx = (ImDrawVert*)m_imgui_vtx[image_index].data();
    uint16_t* idx = (uint16_t*)m_imgui_idx[image_index].data();
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
    m_vertex_buffers[image_index]->update(m_imgui_vtx[image_index].data(), vertex_sz);
    m_index_buffers[image_index]->update(m_imgui_idx[image_index].data(), index_sz);
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