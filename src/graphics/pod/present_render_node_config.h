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

    bool init(std::shared_ptr<VulkanDevice> device, const pugi::xml_node& node_data);

    const std::string& getName() const;
    uint32_t getSCImageIndex() const;
    void setSCImageIndex(uint32_t image_index);
    const std::shared_ptr<VulkanDevice>& getDevice() const;
    // const std::string& getCommandBufferName() const;
    // const std::vector<std::string>& getWaitSemaphoreNames() const;

    std::shared_ptr<PresentRenderNodeConfig> makeInstance(std::string name, uint32_t image_index) const;

private:
    std::shared_ptr<VulkanDevice> m_device;
    uint32_t m_image_index;
    std::string m_name;
};