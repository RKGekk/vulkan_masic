#include "imgui_drawable.h"

#include "../vulkan_renderer.h"
#include "../pod/image_buffer_config.h"
#include "../pod/format_config.h"
#include "../pod/graphics_render_node.h"
#include "../pod/graphics_render_node_config.h"
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

std::shared_ptr<VulkanImageBuffer> ImGUIDrawable::makeFontTexture(std::shared_ptr<VulkanDevice> device, const char* TTF_font_file_name, float fontSizePixels) {
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

    //io.Fonts->TexID = 0u;
    io.FontDefault = font;

    return font_texture;
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

    m_font_texture = makeFontTexture(m_device, TTF_font_file_name.c_str(), font_size_pixels);

    for(size_t frame = 0u; frame < max_frames; ++frame) {
        std::shared_ptr<PerFrameData> per_frame = std::make_shared<PerFrameData>();

        per_frame->imgui_vtx.resize(max_frames);
        per_frame->imgui_idx.resize(max_frames);
        per_frame->vertex_buffer = Application::GetRenderer().getResourcesManager()->create_buffer(nullptr, 0, "imgui_vertex_buffer_frame_"s + std::to_string(frame), "imgui_vertex_resource");
        per_frame->index_buffer = Application::GetRenderer().getResourcesManager()->create_buffer(nullptr, 0, "imgui_index_buffer_frame_"s + std::to_string(frame), "imgui_index_resource");
        per_frame->uniform_buffer = Application::GetRenderer().getResourcesManager()->create_buffer(nullptr, 0, "imgui_uniform_buffer_frame_"s + std::to_string(frame), "imgui_uniform_resource");

        m_per_frame.push_back(per_frame);
    }

    return true;
}

void ImGUIDrawable::reset() {
    
}

void ImGUIDrawable::destroy() {
    // size_t sz = m_renderables.size();
    // for(size_t i = 0; i < sz; ++i) {
    //     std::shared_ptr<Renderable>& renderable = m_renderables[i];
    //     renderable->uniform_buffer->destroy();
    //     renderable->vertex_buffer->destroy();
    //     renderable->index_buffer->destroy();
    //     renderable->font_texture->destroy();
    //     //renderable->render_node->destroy();
    // }
}


std::shared_ptr<GraphicsRenderNode> ImGUIDrawable::makeRenderable(uint32_t image_index) {
    const std::vector<std::shared_ptr<VulkanImageBuffer>>& swapchain_images = Application::GetRenderer().getSwapchain()->getSwapchainImages();

    std::shared_ptr<GraphicsRenderNode> render_node;
    render_node = std::make_shared<GraphicsRenderNode>();
    render_node->init(m_device, "imgui_renderer"s, true, Application::GetRenderer().getFrameData(image_index)->render_graph);

    std::shared_ptr<VulkanShader> vertex_shader = render_node->getPipeline()->getShader(VK_SHADER_STAGE_VERTEX_BIT);
    std::shared_ptr<DescSetLayout> desc_set_layout = Application::GetRenderer().getDescriptorsManager()->getDescSetLayout(vertex_shader->getShaderSignature()->getDescSetNames().at(0));

    render_node->addReadDependency(m_per_frame[image_index]->vertex_buffer, vertex_shader->getShaderSignature()->getVertexFormat().getVertexBufferBindingName());
    render_node->addReadDependency(m_per_frame[image_index]->index_buffer, vertex_shader->getShaderSignature()->getVertexFormat().getIndexBufferBindingName());
    render_node->addReadDependency(m_per_frame[image_index]->uniform_buffer, desc_set_layout->getBindingName(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER));
    render_node->addReadDependency(m_font_texture, desc_set_layout->getBindingName(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER));
    render_node->addWriteDependency(swapchain_images[image_index], "resolve_attachment");
    render_node->addWriteDependency(Application::GetRenderer().getOutColorImage(image_index), "color_attachment");
    render_node->addWriteDependency(Application::GetRenderer().getOutDepthImage(image_index), "depth_attachment");
    render_node->finishRenderNode();

    return render_node;
}

void ImGUIDrawable::update(const GameTimerDelta& delta, uint32_t image_index) {
    std::shared_ptr<PerFrameData>& per_frame = m_per_frame.at(image_index);

    ImDrawData* dd = ImGui::GetDrawData();
    const float L = dd->DisplayPos.x;
    const float R = dd->DisplayPos.x + dd->DisplaySize.x;
    const float T = dd->DisplayPos.y;
    const float B = dd->DisplayPos.y + dd->DisplaySize.y;
    const ImVec2 clip_off = dd->DisplayPos;
    const ImVec2 clip_scale = dd->FramebufferScale;
    const float fb_width = dd->DisplaySize.x * dd->FramebufferScale.x;
    const float fb_height = dd->DisplaySize.y * dd->FramebufferScale.y;

    if(per_frame->imgui_vtx.size() < dd->TotalVtxCount) {
        per_frame->imgui_vtx.resize(dd->TotalVtxCount);
    }

    if(per_frame->imgui_idx.size() < dd->TotalIdxCount) {
        per_frame->imgui_idx.resize(dd->TotalIdxCount);
    }

    int vertex_sz = 0;
    int index_sz = 0;
    uint32_t vtxOffset = 0;
    uint32_t idxOffset = 0;
    ImDrawVert* vtx = (ImDrawVert*)per_frame->imgui_vtx.data();
    uint16_t* idx = (uint16_t*)per_frame->imgui_idx.data();
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

    per_frame->vertex_buffer->update(per_frame->imgui_vtx.data(), vertex_sz);
    per_frame->index_buffer->update(per_frame->imgui_idx.data(), index_sz);

    ImGuiUniformBufferObject bindData;
    bindData.LRTB = {L, R, T, B};
    per_frame->uniform_buffer->update(&bindData, sizeof(ImGuiUniformBufferObject));

    size_t cmd_ct = 0u;
    for (const ImDrawList* cmdList : dd->CmdLists) {
        for (int cmd_i = 0; cmd_i < cmdList->CmdBuffer.Size; cmd_i++) {
            const ImDrawCmd cmd = cmdList->CmdBuffer[cmd_i];

            ImVec2 clipMin((cmd.ClipRect.x - clip_off.x) * clip_scale.x, (cmd.ClipRect.y - clip_off.y) * clip_scale.y);
            ImVec2 clipMax((cmd.ClipRect.z - clip_off.x) * clip_scale.x, (cmd.ClipRect.w - clip_off.y) * clip_scale.y);
            
            if (clipMin.x < 0.0f) clipMin.x = 0.0f;
            if (clipMin.y < 0.0f) clipMin.y = 0.0f;
            if (clipMax.x > fb_width ) clipMax.x = fb_width;
            if (clipMax.y > fb_height) clipMax.y = fb_height;
            if (clipMax.x <= clipMin.x || clipMax.y <= clipMin.y)  continue;

            while(per_frame->renderables.size() <= cmd_ct) {
                std::shared_ptr<GraphicsRenderNode> renderable = makeRenderable(image_index);
                per_frame->renderables.push_back(renderable);
                Application::GetRenderer().addRenderNode(renderable, image_index);
            }
            std::shared_ptr<GraphicsRenderNode>& renderable = per_frame->renderables[cmd_ct];
            renderable->setExecutionBypass(false);
            renderable->setExecutionOrder(cmd_ct);
            
            VkRect2D scissor{};
            scissor.offset = VkOffset2D{(int32_t)clipMin.x, (int32_t)clipMin.y};
            scissor.extent = VkExtent2D{uint32_t(clipMax.x - clipMin.x), uint32_t(clipMax.y - clipMin.y)};

            renderable->getGraphicsRenderNodeConfig()->setScissor(scissor);
            renderable->getGraphicsRenderNodeConfig()->setIndexCount(cmd.ElemCount);
            renderable->getGraphicsRenderNodeConfig()->setFirstIndex(idxOffset + cmd.IdxOffset);
            renderable->getGraphicsRenderNodeConfig()->setVertexOffset(int32_t(vtxOffset + cmd.VtxOffset));

            ++cmd_ct;
        }
        idxOffset += cmdList->IdxBuffer.Size;
        vtxOffset += cmdList->VtxBuffer.Size;
    }

    for(size_t i = cmd_ct; i < per_frame->renderables.size(); ++i) {
        per_frame->renderables[i]->setExecutionBypass(true);
    }

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