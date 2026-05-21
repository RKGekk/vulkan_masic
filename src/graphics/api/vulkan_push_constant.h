#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <cstdint>
#include <string>
#include <memory>
#include <unordered_map>

#include "../pod/render_resource.h"
#include "../pod/push_constant_config.h"

class VulkanDevice;

class VulkanPushConstant : public RenderResource {
public:
    VulkanPushConstant(std::shared_ptr<VulkanDevice> device, std::string name);
    VulkanPushConstant(std::shared_ptr<VulkanDevice> device);

    bool init(std::shared_ptr<PushConstantConfig> const_config);

    void destroy() override;

    const ResourceName& getName() const override;
    Type getType() const override;

    void SetValue(const std::string& name, const void* data);

    const std::vector<char>& getData() const;
    const std::shared_ptr<PushConstantConfig>& getConstConfig() const;

protected:

    std::shared_ptr<VulkanDevice> m_device;
    std::string m_name;
    std::shared_ptr<PushConstantConfig> m_const_config;
	std::vector<char> m_data;
};