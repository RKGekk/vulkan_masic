#include "vulkan_render_pass.h"

#include "vulkan_device.h"
#include "../pod/render_pass_config.h"
#include "../../application.h"

bool VulkanRenderPass::init(std::shared_ptr<VulkanDevice> device, std::shared_ptr<RenderPassConfig> render_pass_cfg) {
    using namespace std::literals;

    m_device = device;
    m_render_pass_cfg = std::move(render_pass_cfg);

    VkResult result = vkCreateRenderPass(m_device->getDevice(), &m_render_pass_cfg->getRenderPassCreateInfo(), nullptr, &m_render_pass);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }

#ifndef NDEBUG
    std::string renderpass_name = "renderpass_"s + m_render_pass_cfg->getName();
    auto vkSetDebugUtilsObjectNameEXT = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetInstanceProcAddr(Application::GetInstance().getInstance(), "vkSetDebugUtilsObjectNameEXT");
    VkDebugUtilsObjectNameInfoEXT name_info = {};
    name_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    name_info.objectType = VK_OBJECT_TYPE_RENDER_PASS;
    name_info.objectHandle = (uint64_t)m_render_pass;
    name_info.pObjectName = renderpass_name.c_str();

    vkSetDebugUtilsObjectNameEXT(m_device->getDevice(), &name_info);
#endif    

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