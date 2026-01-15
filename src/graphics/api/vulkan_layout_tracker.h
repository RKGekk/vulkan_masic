#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "vulkan_image_buffer.h"

#include <memory>
#include <unordered_map>

class VulkanLayoutTracker {
public:

private:
    std::unordered_map<std::shared_ptr<VulkanImageBuffer>, VkImageLayout> m_layout_state;
};