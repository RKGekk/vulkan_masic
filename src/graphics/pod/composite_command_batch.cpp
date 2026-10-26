#include "composite_command_batch.h"

#include "../api/vulkan_command_buffer.h"
#include "../api/vulkan_command_manager.h"
#include "../api/vulkan_framebuffer.h"
#include "../api/vulkan_render_pass.h"
#include "framebuffer_config.h"
#include "render_pass_config.h"

bool CompositeCommandBatch::init(std::shared_ptr<VulkanFramebuffer> fb, const std::shared_ptr<VulkanCommandManager>& cmd_mgr, size_t command_buffer_count, std::string name) {
    using namespace std::literals;

    m_name = std::move(name);
    m_frame_buffer = std::move(fb);
    m_primary_command_buffer = cmd_mgr->allocCommandBufferPtr(PoolTypeEnum::GRAPICS, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1u, name + "_primary"s);
    m_secondary_command_buffer = cmd_mgr->allocCommandBufferPtr(PoolTypeEnum::GRAPICS, VK_COMMAND_BUFFER_LEVEL_SECONDARY, command_buffer_count, name + "_secondary"s);

    return true;
}

void CompositeCommandBatch::beginCommandBuffer(const std::string& subpass_name) const {
    using namespace std::literals;

    VkCommandBufferBeginInfo primary_begin_info = {};
    primary_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    primary_begin_info.pNext = nullptr;
    primary_begin_info.flags = 0u;
    primary_begin_info.pInheritanceInfo = NULL;

    VkResult result = vkBeginCommandBuffer(m_primary_command_buffer->getCommandBufer(), &primary_begin_info);
    if(result != VK_SUCCESS) {
       throw std::runtime_error("failed to begin primary recording commandbuffer !");
    }

    VkCommandBufferInheritanceInfo inherit_info = {};
    inherit_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
    inherit_info.pNext = NULL;
    inherit_info.renderPass = m_frame_buffer->getRenderpass()->getRenderPass();
    inherit_info.subpass = m_frame_buffer->getRenderpass()->getRenderPassConfig()->getSubpassIdx(subpass_name);
    inherit_info.framebuffer = m_frame_buffer->getFramebuffer();
    inherit_info.occlusionQueryEnable = VK_FALSE;
    inherit_info.queryFlags = 0;
    inherit_info.pipelineStatistics = 0;

    VkCommandBufferBeginInfo secondary_begin_info = {};
    secondary_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    secondary_begin_info.pNext = nullptr;
    secondary_begin_info.flags = 0u;
    secondary_begin_info.pInheritanceInfo = &inherit_info;

    size_t sz = m_secondary_command_buffer->getCommandBuferCount();
    for (size_t i = 0u; i < sz; ++i) {
        VkResult result = vkBeginCommandBuffer(m_secondary_command_buffer->getCommandBufer(i), &secondary_begin_info);
        if(result != VK_SUCCESS) {
           throw std::runtime_error("failed to begin recording commandbuffer!");
        }
    }
    
    VkRenderPassBeginInfo renderpass_info{};
    renderpass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderpass_info.renderPass = m_frame_buffer->getRenderpass()->getRenderPass();
    renderpass_info.framebuffer = m_frame_buffer->getFramebuffer();
    renderpass_info.renderArea.offset = m_frame_buffer->getFramebufferConfig()->getOffset2D();
    renderpass_info.renderArea.extent = m_frame_buffer->getFramebufferConfig()->getExtent2D();
    renderpass_info.clearValueCount = m_frame_buffer->getRenderpass()->getRenderPassConfig()->getClearValues().size();
    renderpass_info.pClearValues = m_frame_buffer->getRenderpass()->getRenderPassConfig()->getClearValues().data();

    vkCmdBeginRenderPass(m_primary_command_buffer->getCommandBufer(), &renderpass_info, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
}

void CompositeCommandBatch::endCommandBuffer() const {
    vkCmdExecuteCommands(m_primary_command_buffer->getCommandBufer(), m_secondary_command_buffer->getCommandBuferCount(), m_secondary_command_buffer->getCommandBuferPtr());
    vkCmdEndRenderPass(m_primary_command_buffer->getCommandBufer());
    vkCmdEndRenderPass(m_primary_command_buffer->getCommandBufer());
}


const std::shared_ptr<CommandBatch>& CompositeCommandBatch::getPrimaryCommandBuffer() const {
    return m_primary_command_buffer;
}

const std::shared_ptr<CommandBatch>& CompositeCommandBatch::getSecondaryCommandBuffer() const {
    return m_secondary_command_buffer;
}

const std::shared_ptr<VulkanFramebuffer>& CompositeCommandBatch::getFrameBuffer() const {
    return m_frame_buffer;
}
