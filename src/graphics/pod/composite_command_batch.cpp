#include "composite_command_batch.h"

#include "../api/vulkan_command_buffer.h"
#include "../api/vulkan_command_manager.h"
#include "../api/vulkan_framebuffer.h"
#include "../api/vulkan_render_pass.h"
#include "framebuffer_config.h"
#include "render_pass_config.h"
#include "../../application.h"
#include "../api/vulkan_command_manager.h"

bool CompositeCommandBatch::init(std::shared_ptr<VulkanFramebuffer> fb, size_t frame_id, size_t thread_id, size_t command_buffer_count, std::string name) {
    using namespace std::literals;

    m_command_manager = Application::Get().GetRenderer().getCommandManager();

    m_name = std::move(name);
    m_frame_buffer = std::move(fb);
    m_primary_command_buffer = m_command_manager->allocCommandBufferPtr(PoolTypeEnum::GRAPICS, VK_COMMAND_BUFFER_LEVEL_PRIMARY, frame_id, thread_id, 1u, m_name + "_primary"s);
    m_secondary_command_buffer = m_command_manager->allocCommandBufferPtr(PoolTypeEnum::GRAPICS, VK_COMMAND_BUFFER_LEVEL_SECONDARY, frame_id, thread_id, command_buffer_count, m_name + "_secondary"s);

    m_recorded_primary = false;
    m_recorded_secondary = std::vector<bool>(command_buffer_count, false);

    return true;
}

void CompositeCommandBatch::beginPrimaryCommandBuffer(const ResourcesStateTransitionFn& transition_fn) {
    using namespace std::literals;

    m_recorded_primary = false;

    VkCommandBufferBeginInfo primary_begin_info = {};
    primary_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    primary_begin_info.pNext = nullptr;
    primary_begin_info.flags = 0u;
    primary_begin_info.pInheritanceInfo = NULL;

    m_primary_command_buffer->beginCommandBuffer(&primary_begin_info);

    transition_fn(m_primary_command_buffer->getCommandBufer());
    
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

void CompositeCommandBatch::beginSecondaryCommandBuffer(const std::string& subpass_name, size_t index) {
    using namespace std::literals;

    m_recorded_secondary[index] = false;

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

    m_secondary_command_buffer->beginCommandBuffer(&secondary_begin_info, index);
}

void CompositeCommandBatch::endPrimaryCommandBuffer() {
    vkCmdExecuteCommands(m_primary_command_buffer->getCommandBufer(), m_secondary_command_buffer->getCommandBuferCount(), m_secondary_command_buffer->getCommandBuferPtr());
    vkCmdEndRenderPass(m_primary_command_buffer->getCommandBufer());
    m_primary_command_buffer->endCommandBuffer();

    m_recorded_primary = true;
}

void CompositeCommandBatch::endSecondaryCommandBuffer(size_t index) {
    m_secondary_command_buffer->endCommandBuffer();
    
    if(index != CommandBatch::SELECT_ALL_BUFFERS) {
        m_recorded_secondary[index] = true;
    }
    else {
        size_t sz = m_secondary_command_buffer->getCommandBuferCount();
        for (size_t i = 0u; i < sz; ++i) {
            m_recorded_secondary[i] = true;
        }
    }
}

void CompositeCommandBatch::resetCommandBuffer(VkCommandBufferLevel lvl, size_t index = 0u) {
    if(lvl == VK_COMMAND_BUFFER_LEVEL_PRIMARY) {
        if(m_command_manager->getPrimaryCmdPoolFlags() & VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT) {
            m_primary_command_buffer->reset();
        }
        else {
            m_command_manager->resetPool(index, PoolTypeEnum::GRAPICS, lvl);
        }
        m_recorded_primary = false;
    }
    else {
        if(m_command_manager->getSecondaryCmdPoolFlags() & VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT) {
            m_secondary_command_buffer->reset();
        }
        else {
            m_command_manager->resetPool(index, PoolTypeEnum::GRAPICS, lvl);
        }
    }
}

bool CompositeCommandBatch::isRecorded(VkCommandBufferLevel lvl, size_t index = CommandBatch::SELECT_ALL_BUFFERS) const {

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
