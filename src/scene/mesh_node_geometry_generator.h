#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/transform.hpp>

#include "scene.h"
#include "nodes/scene_node.h"
#include "../graphics/pod/material.h"
#include "../graphics/pod/shader_signature.h"
#include "../graphics/api/vulkan_shaders_manager.h"
#include "../animation/matrix_animation.h"

class MeshNodeGeometryGenerator {
public:
	MeshNodeGeometryGenerator() = default;

    //std::shared_ptr<SceneNode> GenerateSceneNodeLine(std::shared_ptr<VulkanShadersManager> shader_manager, std::shared_ptr<SceneNode> root_transform);
    //std::shared_ptr<SceneNode> GenerateSceneNodeSpline(std::shared_ptr<VulkanShadersManager> shader_manager, std::shared_ptr<SceneNode> root_transform);
    std::shared_ptr<SceneNode> GenerateSceneNodeSpline(const std::string& mesh_name, float line_width, const std::vector<KeyframeMatrixTranslation>& keyframes, size_t points_per_spline, std::shared_ptr<VulkanShadersManager> shader_manager, std::shared_ptr<SceneNode> root_transform);
    //std::shared_ptr<SceneNode> GenerateSceneNodeBox(std::shared_ptr<VulkanShadersManager> shader_manager, std::shared_ptr<SceneNode> root_transform);
    //std::shared_ptr<SceneNode> GenerateSceneNodeSphere(std::shared_ptr<VulkanShadersManager> shader_manager, std::shared_ptr<SceneNode> root_transform);
    //std::shared_ptr<SceneNode> GenerateSceneNodeGeosphere(std::shared_ptr<VulkanShadersManager> shader_manager, std::shared_ptr<SceneNode> root_transform);
    //std::shared_ptr<SceneNode> GenerateSceneNodeCylinder(std::shared_ptr<VulkanShadersManager> shader_manager, std::shared_ptr<SceneNode> root_transform);
    //std::shared_ptr<SceneNode> GenerateSceneNodeGrid(std::shared_ptr<VulkanShadersManager> shader_manager, std::shared_ptr<SceneNode> root_transform);
    //std::shared_ptr<SceneNode> GenerateSceneNodeQuad(std::shared_ptr<VulkanShadersManager> shader_manager, std::shared_ptr<SceneNode> root_transform);

private:
    std::shared_ptr<VulkanDevice> m_device;
    std::shared_ptr<Scene> m_scene;
    std::shared_ptr<SceneNode> m_root_node;
    std::shared_ptr<VulkanShadersManager> m_shader_manager;
    std::string m_default_vertex_shader_name;
};
