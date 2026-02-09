#include "vulkan_render_pass.h"

#include "vulkan_device.h"
#include "../pod/render_pass_config.h"

bool VulkanRenderPass::init(std::shared_ptr<VulkanDevice> device, std::shared_ptr<RenderPassConfig> render_pass_cfg) {
    m_device = device;
    m_render_pass_cfg = render_pass_cfg;

    VkResult result = vkCreateRenderPass(m_device->getDevice(), &m_render_pass_cfg->getRenderPassCreateInfo(), nullptr, &m_render_pass);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }

    return true;
}

void VulkanRenderPass::destroy() {
    vkDestroyRenderPass(m_device->getDevice(), m_render_pass, nullptr);
}

VkRenderPass VulkanRenderPass::getRenderPass() const {
    return m_render_pass;
}

const std::shared_ptr<RenderPassConfig>& VulkanRenderPass::getRenderPassConfig() const {
    return m_render_pass_cfg;
}