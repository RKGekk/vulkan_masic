#include "frame_copy_command.h"

#include "../api/vulkan_command_buffer.h"
#include "../../application.h"
#include "../vulkan_renderer.h"
#include "../api/vulkan_swapchain.h"

FrameCopyCommand::FrameCopyCommand(std::string cpy_cmd_name) {
    VulkanRenderer& renderer = Application::GetRenderer();
    std::shared_ptr<VulkanCommandManager>& cmd_manager = renderer.getCommandManager();
    const std::shared_ptr<VulkanSwapChain>& swapchain = renderer.getSwapchain();
    m_max_frames = swapchain->getMaxFrames();
    for(int i = 0; i < m_max_frames; ++i) {
        m_cpy_cmd[i] = cmd_manager->allocCommandBufferPtr(PoolTypeEnum::TRANSFER, VK_COMMAND_BUFFER_LEVEL_PRIMARY, i, 0u, 1u, cpy_cmd_name + std::to_string(i));
    }
}

bool FrameCopyCommand::UpdateFameData(uint32_t image_index, size_t thread_id) {
    VulkanRenderer& renderer = Application::GetRenderer();
    std::shared_ptr<PerFrame>& per_frame = renderer.getFrameData(image_index);
    std::shared_ptr<CommandBatch>& cpy_cmd = m_cpy_cmd[image_index];
    cpy_cmd->reset();

    bool res = CalcFameData(cpy_cmd, image_index, thread_id);

    per_frame->wait_transfer_commands.push_back(cpy_cmd);
    return res;
}

int FrameCopyCommand::getMaxFrames() const {
    return m_max_frames;
}
