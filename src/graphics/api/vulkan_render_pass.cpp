#include "vulkan_render_pass.h"

#include "vulkan_device.h"

bool VulkanRenderPass::init(std::shared_ptr<VulkanDevice> device, const pugi::xml_node& render_pass_data) {
    m_device = device;

    return true;
}