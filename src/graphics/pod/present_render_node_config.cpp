#include "present_render_node_config.h"

#include "../api/vulkan_device.h"
#include "../api/vulkan_sampler.h"
#include "../api/vulkan_resources_manager.h"
#include "../api/vulkan_pipeline.h"
#include "../api/vulkan_pipelines_manager.h"
#include "../../window_surface.h"
#include "../api/vulkan_swapchain.h"
#include "image_buffer_config.h"
#include "format_config.h"

bool PresentRenderNodeConfig::init(const std::shared_ptr<VulkanDevice>& device, uint32_t image_index, const pugi::xml_node& node_data) {
    using namespace std::literals;

    image_index = image_index;
    m_name = node_data.attribute("name").as_string();

    pugi::xml_node wait_semaphores_node = node_data.child("wait_semaphores");
    if(wait_semaphores_node) {
        for (pugi::xml_node wait_semaphore_node_node = wait_semaphores_node.first_child(); wait_semaphore_node_node; wait_semaphore_node_node = wait_semaphore_node_node.next_sibling()) {
            m_wait_semaphore_names.push_back(wait_semaphore_node_node.text().as_string());
        }
    }

    pugi::xml_node command_buffer_settings_node = node_data.child("CommandBufferSettings");
    if(command_buffer_settings_node) {
        m_command_buffer_name = command_buffer_settings_node.child("command_buffer_name").text().as_string();
    }

    return true;
}


const std::string& PresentRenderNodeConfig::getName() const {
    return m_name;
}

const std::string& PresentRenderNodeConfig::getCommandBufferName() const {
    return m_command_buffer_name;
}

const std::vector<std::string>& PresentRenderNodeConfig::getWaitSemaphoreNames() const {
    return m_wait_semaphore_names;
}

std::shared_ptr<PresentRenderNodeConfig> PresentRenderNodeConfig::makeInstance(std::string name) const {
    std::shared_ptr<PresentRenderNodeConfig> instance = std::make_shared<PresentRenderNodeConfig>();
    *instance = *this;
    instance->m_name = std::move(name);

    return instance;
}