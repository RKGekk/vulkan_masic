#include "vulkan_renderer.h"

#include "api/vulkan_buffer.h"
#include "api/vulkan_command_buffer.h"
#include "api/vulkan_command_manager.h"
#include "api/vulkan_descriptors_manager.h"
#include "api/vulkan_device.h"
#include "api/vulkan_framebuffer.h"
#include "api/vulkan_fence_manager.h"
#include "api/vulkan_format_manager.h"
#include "api/vulkan_image_buffer.h"
#include "api/vulkan_pipelines_manager.h"
#include "api/vulkan_pipeline.h"
#include "api/vulkan_render_pass.h"
#include "api/vulkan_render_passes_manager.h"
#include "api/vulkan_resources_manager.h"
#include "api/vulkan_swapchain.h"
#include "api/vulkan_shader.h"
#include "api/vulkan_shaders_manager.h"
#include "api/vulkan_semaphores_manager.h"
#include "pod/composite_command_batch.h"
#include "pod/render_node.h"
#include "pod/present_render_node.h"
#include "pod/graphics_render_node.h"
#include "pod/render_graph.h"
#include "pod/framebuffer_config.h"
#include "pod/graphics_render_node_config.h"
#include "pod/present_render_node_config.h"
#include "pod/image_buffer_config.h"
#include "pod/render_pass_config.h"
#include "pod/pipeline_config.h"
#include "../scene/light_manager.h"

#include <algorithm>
#include <thread>

void PerFrame::destroy(VulkanRenderer& renderer) {
    renderer.getResourcesManager()->delete_image(out_color_image);
    renderer.getResourcesManager()->delete_image(out_depth_image);
    present_render_node->destroy();
    render_graph->destroy();
}
	
bool VulkanRenderer::init(std::shared_ptr<VulkanDevice> device, std::shared_ptr<WindowSurface> window, std::shared_ptr<ThreadPool> thread_pool) {
    using namespace std::literals;

    m_device = std::move(device);
    m_thread_pool = std::move(thread_pool);

    m_format_manager = std::make_shared<VulkanFormatManager>();
    m_format_manager->init(m_device, window, "graphics_pipelines.xml"s);

    m_fence_manager = std::make_shared<VulkanFenceManager>();
    m_fence_manager->init(m_device);

    m_semaphore_manager = std::make_shared<VulkanSemaphoresManager>();
    m_semaphore_manager->init(m_device);

    m_resources_manager = std::make_shared<VulkanResourcesManager>(m_device, m_format_manager);
    m_resources_manager->init(window, "graphics_pipelines.xml"s);

    m_swapchain = std::make_shared<VulkanSwapChain>();
    m_swapchain->init(m_device, window, m_resources_manager, m_format_manager, "graphics_pipelines.xml"s);

    m_command_manager = std::make_shared<VulkanCommandManager>();
    m_command_manager->init(m_device, m_swapchain->getSwapchainParams().minImageCount, 1u /*std::thread::hardware_concurrency()*/);

    m_descriptors_manager = std::make_shared<VulkanDescriptorsManager>();
    m_descriptors_manager->init(m_device, "graphics_pipelines.xml"s);

    m_render_passes_manager = std::make_shared<VulkanRenderPassesManager>();
    m_render_passes_manager->init(m_device, "graphics_pipelines.xml"s, m_swapchain);

    m_shaders_manager = std::make_shared<VulkanShadersManager>();
    m_shaders_manager->init(m_device, m_resources_manager, "graphics_pipelines.xml"s);

    m_pipelines_manager = std::make_shared<VulkanPipelinesManager>();
    m_pipelines_manager->init(m_device, "graphics_pipelines.xml"s);

    m_frame = 0u;

    const std::vector<std::shared_ptr<VulkanImageBuffer>>& swapchain_images = m_swapchain->getSwapchainImages();
    int max_frames = m_swapchain->getMaxFrames();
    m_per_frame.reserve(max_frames);
    for(int i = 0; i < max_frames; ++i) {
        std::shared_ptr<PerFrame> per_frame = std::make_shared<PerFrame>();

        per_frame->out_color_image = m_resources_manager->create_image("render_target_color"s + std::to_string(i), "render_target_color_resource");
        per_frame->out_depth_image = m_resources_manager->create_image("render_target_depth"s + std::to_string(i), "render_target_depth_resource");

        //per_frame->swapchain_available_sem = m_semaphore_manager->getSemaphore("swapchain_"s + std::to_string(i) + "_available_sem");
        std::shared_ptr<VulkanSemaphore> spare_swapchain_available_sem = m_semaphore_manager->getSemaphore("swapchain_"s + std::to_string(i) + "_available_spare_sem");
        m_spare_swapchain_available_sem.push_back(spare_swapchain_available_sem);

        //per_frame->swapchain_available_fen = m_fence_manager->getFence("swapchain_"s + std::to_string(i) + "_available_fen"s);
        std::shared_ptr<VulkanFence> spare_swapchain_available_fen = m_fence_manager->getFence("swapchain_"s + std::to_string(i) + "_available_spare_fen"s);
        m_spare_swapchain_available_fen.push_back(spare_swapchain_available_fen);

        per_frame->transfer_complete_sem = m_semaphore_manager->getSemaphore("transfer_"s + std::to_string(i) + "_complete_sem");
        per_frame->transfer_complete_fen = m_fence_manager->getFence("transfer_"s + std::to_string(i) + "_complete_sem"s);
        per_frame->cmd_submit_finish_signal_sem = m_semaphore_manager->getSemaphore("submit_finish_"s + std::to_string(i) + "_signal_sem");
        per_frame->cmd_submit_finish_fence = m_fence_manager->getFence("submit_finish_"s + std::to_string(i) + "_fen"s);

        per_frame->render_graph = std::make_shared<RenderGraph>();
        per_frame->render_graph->init(m_device, window, i);

        std::shared_ptr<PresentRenderNodeConfig> present_cfg = per_frame->render_graph->makePresentRenderNodeCfgInstance("presenter"s, "PresentRenderNode"s + std::to_string(i));
        per_frame->present_render_node = std::make_shared<PresentRenderNode>();
        per_frame->present_render_node->init("PresentRenderNode"s + std::to_string(i), present_cfg, per_frame->render_graph);
        per_frame->present_render_node->addReadDependency(swapchain_images.at(i), "swapchain_image"s);
        per_frame->render_graph->add_pass(per_frame->present_render_node);

        per_frame->copy_command = m_command_manager->allocCommandBufferPtr(PoolTypeEnum::TRANSFER, VK_COMMAND_BUFFER_LEVEL_PRIMARY, i, 0u, 1u, "cpy_buffer_frame_"s + std::to_string(i));
        
        per_frame->frame_index = i;

        m_per_frame.push_back(std::move(per_frame));
    }

    return true;
}

void VulkanRenderer::destroy() {
    vkDeviceWaitIdle(m_device->getDevice());

    m_descriptors_manager->destroy();

    size_t sz = m_swapchain->getMaxFrames();
    m_swapchain->destroy();
    for(size_t i = 0u; i < sz; ++i) {
        m_per_frame[i]->destroy(*this);
    }
    m_command_manager->destroy();
    m_fence_manager->destroy();
    m_semaphore_manager->destroy();
    m_resources_manager->destroy();
    m_shaders_manager->destroy();
    m_pipelines_manager->destroy();
    m_render_passes_manager->destroy();
}

const std::shared_ptr<VulkanSwapChain>& VulkanRenderer::getSwapchain() const {
    return m_swapchain;
}

std::shared_ptr<VulkanDevice>& VulkanRenderer::GetDevice() {
    return m_device;
}

std::shared_ptr<VulkanDescriptorsManager>& VulkanRenderer::getDescriptorsManager() {
    return m_descriptors_manager;
}

std::shared_ptr<VulkanShadersManager>& VulkanRenderer::getShadersManager() {
    return m_shaders_manager;
}

std::shared_ptr<VulkanPipelinesManager>& VulkanRenderer::getPipelinesManager() {
    return m_pipelines_manager;
}
	
std::shared_ptr<VulkanFenceManager>& VulkanRenderer::getFenceManager() {
    return m_fence_manager;
}

std::shared_ptr<VulkanSemaphoresManager>& VulkanRenderer::getSemaphoreManager() {
    return m_semaphore_manager;
}

std::shared_ptr<VulkanCommandManager>& VulkanRenderer::getCommandManager() {
    return m_command_manager;
}

std::shared_ptr<VulkanRenderPassesManager>& VulkanRenderer::getRenderPassesManager() {
    return m_render_passes_manager;
}

std::shared_ptr<VulkanFormatManager>& VulkanRenderer::getFormatManager() {
    return m_format_manager;
}

std::shared_ptr<VulkanResourcesManager>& VulkanRenderer::getResourcesManager() {
    return m_resources_manager;
}

std::shared_ptr<VulkanImageBuffer>& VulkanRenderer::getOutColorImage(uint32_t image_index) {
    return m_per_frame[image_index]->out_color_image;
}

std::shared_ptr<VulkanImageBuffer>& VulkanRenderer::getOutDepthImage(uint32_t image_index) {
    return m_per_frame[image_index]->out_depth_image;
}

std::shared_ptr<VulkanBuffer> VulkanRenderer::makeTransferCommand(std::string buffer_name, std::string buffer_type, const void* data_ptr, size_t buffer_size, uint32_t frame_index) {
    using namespace std::literals;
	std::shared_ptr<CpuResource> vertex_cpu_buff = m_resources_manager->create_cpu_buffer(buffer_name + "_cpu_data_buffer"s, buffer_size, data_ptr);
    return makeTransferCommand(std::move(buffer_name), std::move(buffer_type), std::move(vertex_cpu_buff), frame_index);
}

std::shared_ptr<VulkanBuffer> VulkanRenderer::makeTransferCommand(std::string buffer_name, std::string buffer_type, std::shared_ptr<CpuResource> cpu_resource, uint32_t frame_index) {
    using namespace std::literals;
    size_t import_cpy_cmd_frame = frame_index == LAST_AVAILABLE_FRAME ? getLastFrame() : frame_index;
    std::string vtx_cpy_cmd_name = "cpy_cmd_"s + buffer_name + "_frame"s + std::to_string(import_cpy_cmd_frame);
	std::shared_ptr<CommandBatch> copy_cmd = m_command_manager->allocCommandBufferPtr(PoolTypeEnum::TRANSFER, VK_COMMAND_BUFFER_LEVEL_PRIMARY, import_cpy_cmd_frame, 0u, 1u, vtx_cpy_cmd_name);
    copy_cmd->beginCommandBuffer();
    std::shared_ptr<VulkanBuffer> data_buffer = m_resources_manager->create_buffer(buffer_name, buffer_type, std::move(cpu_resource), copy_cmd);
    copy_cmd->endCommandBuffer();
    m_per_frame[import_cpy_cmd_frame]->wait_transfer_commands.push_back(copy_cmd);

    return data_buffer;
}

std::shared_ptr<VulkanImageBuffer> VulkanRenderer::makeTransferImgCommand(const std::string& path_to_file, uint32_t frame_index) {
    using namespace std::literals;
    size_t import_cpy_cmd_frame = frame_index == LAST_AVAILABLE_FRAME ? getLastFrame() : frame_index;
    std::string texture_cpy_cmd_name = "image_cpy_cmd_"s + path_to_file + "_frame"s + std::to_string(import_cpy_cmd_frame);
	std::shared_ptr<CommandBatch> copy_cmd = m_command_manager->allocCommandBufferPtr(PoolTypeEnum::TRANSFER, VK_COMMAND_BUFFER_LEVEL_PRIMARY, import_cpy_cmd_frame, 0u, 1u, texture_cpy_cmd_name);
    copy_cmd->beginCommandBuffer();
	std::shared_ptr<VulkanImageBuffer> image_buffer = m_resources_manager->create_image(path_to_file, copy_cmd);
    copy_cmd->endCommandBuffer();
	m_per_frame[import_cpy_cmd_frame]->wait_transfer_commands.push_back(std::move(copy_cmd));

    return image_buffer;
}

std::shared_ptr<VulkanImageBuffer> VulkanRenderer::makeTransferImgCommand(VkExtent2D extent, std::shared_ptr<CpuResource> cpu_resource, std::string image_name, std::string resource_type_name, uint32_t frame_index) {
    using namespace std::literals;
    size_t import_cpy_cmd_frame = frame_index == LAST_AVAILABLE_FRAME ? getLastFrame() : frame_index;
    std::string texture_cpy_cmd_name = "image_cpy_cmd_"s + image_name + "_frame"s + std::to_string(import_cpy_cmd_frame);
	std::shared_ptr<CommandBatch> copy_cmd = m_command_manager->allocCommandBufferPtr(PoolTypeEnum::TRANSFER, VK_COMMAND_BUFFER_LEVEL_PRIMARY, import_cpy_cmd_frame, 0u, 1u, texture_cpy_cmd_name);
    copy_cmd->beginCommandBuffer();
	std::shared_ptr<VulkanImageBuffer> image_buffer = m_resources_manager->create_image(std::move(cpu_resource), copy_cmd, extent, image_name, resource_type_name);
    copy_cmd->endCommandBuffer();
    m_per_frame[import_cpy_cmd_frame]->wait_transfer_commands.push_back(std::move(copy_cmd));

    return image_buffer;
}

std::shared_ptr<VulkanImageBuffer> VulkanRenderer::makeTransferImgCommand(VkExtent2D extent, const void* data_ptr, size_t buffer_size, std::string image_name, std::string resource_type_name, uint32_t frame_index) {
    using namespace std::literals;
    std::string cpu_res_name = image_name + "_cpu_resource"s;
	std::shared_ptr<CpuResource> cpu_res = m_resources_manager->create_cpu_buffer(std::move(cpu_res_name), buffer_size, data_ptr);
    return makeTransferImgCommand(extent, std::move(cpu_res), std::move(image_name), std::move(resource_type_name), frame_index);
}

void VulkanRenderer::updateCommand(const std::shared_ptr<VulkanBuffer>& data_buffer, const void* data_ptr, size_t buffer_size, uint32_t frame_index) {
    using namespace std::literals;
    std::shared_ptr<CpuResource> vertex_cpu_buff = m_resources_manager->create_cpu_buffer(data_buffer->getName() + "_cpu_data_buffer"s, buffer_size, data_ptr);
    
    updateCommand(data_buffer, std::move(vertex_cpu_buff), frame_index);
}

void VulkanRenderer::updateCommand(const std::shared_ptr<VulkanBuffer>& data_buffer, std::shared_ptr<CpuResource> cpu_resource, uint32_t frame_index) {
    size_t cpy_cmd_frame = frame_index == LAST_AVAILABLE_FRAME ? getLastFrame() : frame_index;
    const std::shared_ptr<PerFrame>& per_frame = m_per_frame[cpy_cmd_frame];
    const std::shared_ptr<CommandBatch>& cpy_command = per_frame->copy_command;
    cpy_command->addResource(data_buffer);
    data_buffer->update(cpy_command, cpu_resource->getDataPtr(), cpu_resource->getSize());
}

void VulkanRenderer::beginFrame(unsigned image_index) {
    const std::shared_ptr<PerFrame>& per_frame = m_per_frame[image_index];

    if(!per_frame->transfer_complete_fen->isFreeForUse()) {
        per_frame->transfer_complete_fen->wait();
        per_frame->transfer_complete_fen->reset();
    }
    for(const std::shared_ptr<CommandBatch>& transfer_cmd : per_frame->wait_transfer_commands) {
        for(const std::shared_ptr<RenderResource>& transfer_res : transfer_cmd->getResources()) {
            for(const auto& transfer_finish_fn : transfer_res->getTransferFinishFn()) {
                transfer_finish_fn(transfer_res);
            }
        }
    }
    per_frame->wait_transfer_commands.clear();
    per_frame->copy_command->reset();
    per_frame->copy_command->beginCommandBuffer();

    if(!per_frame->cmd_submit_finish_fence->isFreeForUse()) {
        per_frame->cmd_submit_finish_fence->wait();
        per_frame->cmd_submit_finish_fence->reset();
    }
    per_frame->cmd_submit_wait_sem.clear();
    
    per_frame->present_render_node->clearWaitSemaphores();
    per_frame->present_render_node->addWaitSemaphore(per_frame->cmd_submit_finish_signal_sem->getSemaphore());
}

void VulkanRenderer::recordCommandBuffer(unsigned image_index) {
    using namespace std::literals;

    const std::shared_ptr<PerFrame>& per_frame_data = m_per_frame[image_index];
    per_frame_data->submission_buffers.clear();

    const std::vector<std::shared_ptr<DependencyLevel>>& dependency_levels = m_per_frame[image_index]->render_graph->getDependencyLevels();
    for (auto rev_it = dependency_levels.crbegin(); rev_it != dependency_levels.crend(); ++rev_it) {
        const std::shared_ptr<DependencyLevel>& dependency_lvl = *rev_it;
        dependency_lvl->recordCommandBuffer();
        const std::map<size_t, std::unordered_set<std::shared_ptr<VulkanFramebuffer>>>& order_to_fb_map = dependency_lvl->getOrderToFBmap();
        if(order_to_fb_map.empty() && dependency_lvl->getNodes().size() > 0u) {
            dependency_lvl->TransitionResourcesToProperState(per_frame_data->copy_command->getCommandBufer());
            continue;
        }
        for (const auto&[rp_order, fb_set] : order_to_fb_map) {
            for(const std::shared_ptr<VulkanFramebuffer>& fb : fb_set) {
                const std::shared_ptr<CompositeCommandBatch> cmd_batch = dependency_lvl->getFBtoCmdBufferMap().at(fb);
                per_frame_data->submission_buffers.push_back(cmd_batch->getPrimaryCommandBuffer()->getCommandBufer());
            }
        }
    }
}

void VulkanRenderer::drawFrame(unsigned image_index) {
    recordCommandBuffer(image_index);
    update_frame(image_index);

    std::shared_ptr<PerFrame>& per_frame = m_per_frame[image_index];
    
    std::shared_ptr<VulkanCommandManager::BatchWaitInfo> wait_info = std::make_shared<VulkanCommandManager::BatchWaitInfo>();
    if(per_frame->cmd_submit_wait_sem.size()) {
        wait_info->wait_for_semaphores.insert(wait_info->wait_for_semaphores.end(), per_frame->cmd_submit_wait_sem.begin(), per_frame->cmd_submit_wait_sem.end());
    }
    wait_info->wait_for_stages.insert(wait_info->wait_for_stages.cend(), wait_info->wait_for_semaphores.size(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
    wait_info->signal_fence = per_frame->cmd_submit_finish_fence->getFence();
    wait_info->signal_semaphores.push_back(per_frame->cmd_submit_finish_signal_sem->getSemaphore());

    m_command_manager->submitCommandBuffer(m_per_frame[image_index]->submission_buffers, PoolTypeEnum::GRAPICS, wait_info);
    per_frame->cmd_submit_finish_fence->setOccupiedState();

    m_per_frame[image_index]->present_render_node->present();
    //renderer.getSemaphoreManager()->returnSemaphore(swapchain_available_sem);
    //swapchain_available_sem = renderer.getSemaphoreManager()->getSemaphore("swapchain_available_sem");
}

std::shared_ptr<PerFrame>& VulkanRenderer::getFrameData(uint32_t image_index) {
    return m_per_frame[image_index];
}

void VulkanRenderer::update_frame(uint32_t image_index) {
    std::shared_ptr<PerFrame>& per_frame = m_per_frame[image_index];

    per_frame->cmd_submit_wait_sem.push_back(per_frame->transfer_complete_sem->getSemaphore());

    std::shared_ptr<VulkanCommandManager::BatchWaitInfo> wait_info = std::make_shared<VulkanCommandManager::BatchWaitInfo>();
    wait_info->wait_for_semaphores.push_back(per_frame->swapchain_available_sem->getSemaphore());
    wait_info->signal_semaphores.push_back(per_frame->transfer_complete_sem->getSemaphore());
    wait_info->signal_fence = per_frame->transfer_complete_fen->getFence();
    wait_info->wait_for_stages.insert(wait_info->wait_for_stages.cend(), wait_info->wait_for_semaphores.size(), VK_PIPELINE_STAGE_TRANSFER_BIT);
    
    per_frame->copy_command->endCommandBuffer();
    per_frame->transfer_submission_buffers.clear();
    per_frame->transfer_submission_buffers.push_back(per_frame->copy_command->getCommandBufer());
    for(const std::shared_ptr<CommandBatch>& transfer_batch : per_frame->wait_transfer_commands) {
        per_frame->transfer_submission_buffers.push_back(transfer_batch->getCommandBufer());
    }

    m_command_manager->submitCommandBuffer(per_frame->transfer_submission_buffers, PoolTypeEnum::TRANSFER, wait_info);
    per_frame->transfer_complete_fen->setOccupiedState();
}

void VulkanRenderer::addRenderNode(std::shared_ptr<RenderNode> render_node, unsigned image_index) {
    m_per_frame[image_index]->render_graph->add_pass(std::move(render_node));
}

bool try_once() {
    static std::once_flag flag;
    bool is_first = false;
    
    std::call_once(flag, [&]() {
        is_first = true;
    });
    
    return is_first;
}

std::pair<bool, uint32_t> VulkanRenderer::acquire_next_image() {
    bool first_frame = try_once();

    uint32_t min_image_count = m_swapchain->getSwapchainParams().minImageCount;
    if(m_prev_frame.size() >= min_image_count) {
        m_per_frame[m_prev_frame.front()]->swapchain_available_fen->wait();
        m_per_frame[m_prev_frame.front()]->swapchain_available_fen->reset();
        m_spare_swapchain_available_fen.push_back(std::move(m_per_frame[m_prev_frame.front()]->swapchain_available_fen));
        m_spare_swapchain_available_sem.push_back(std::move(m_per_frame[m_prev_frame.front()]->swapchain_available_sem));
        m_prev_frame.erase(m_prev_frame.begin());
    }
    
    if(!first_frame) {
        m_prev_frame.push_back(m_frame);
    }

    std::shared_ptr<VulkanSemaphore> swapchain_available_sem = std::move(m_spare_swapchain_available_sem.back());
    m_spare_swapchain_available_sem.pop_back();

    std::shared_ptr<VulkanFence> swapchain_available_fen = std::move(m_spare_swapchain_available_fen.back());
    m_spare_swapchain_available_fen.pop_back();

    VkResult result = vkAcquireNextImageKHR(
        m_device->getDevice(),
        m_swapchain->getSwapchain(),
        UINT64_MAX,
        swapchain_available_sem->getSemaphore(),
        swapchain_available_fen->getFence(),
        &m_frame
    );
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || result == VK_NOT_READY) {
        return {false, m_frame};
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    m_per_frame[m_frame]->swapchain_available_sem = std::move(swapchain_available_sem);
    m_per_frame[m_frame]->swapchain_available_fen = std::move(swapchain_available_fen);
    m_per_frame[m_frame]->swapchain_available_fen->setOccupiedState();

    return {true, m_frame};
}

uint32_t VulkanRenderer::getLastFrame() const {
    return m_frame;
}

uint32_t VulkanRenderer::getPrevFrame() const {
    return m_prev_frame.back();
    //return m_prev_frame.empty() ? m_swapchain->getMaxFrames() - 1u : m_prev_frame.back();
    //return m_prev_frame.empty() ? 0u : m_prev_frame.back();
}