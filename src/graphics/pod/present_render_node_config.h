#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <pugixml.hpp>

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "render_resource.h"

class VulkanDevice;

class PresentRenderNodeConfig {
public:

    bool init(const std::shared_ptr<VulkanDevice>& device, uint32_t image_index, const pugi::xml_node& node_data);

    const std::string& getName() const;
    const std::string& getCommandBufferName() const;
    const std::vector<std::string>& getWaitSemaphoreNames() const;

    std::shared_ptr<PresentRenderNodeConfig> makeInstance(std::string name) const;

private:
    uint32_t m_image_index;
    std::string m_name;
};