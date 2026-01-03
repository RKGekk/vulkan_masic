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

#include <imgui.h>

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "../../scene/nodes/scene_node.h"
#include "../../scene/nodes/mesh_node.h"
#include "../../scene/nodes/camera_node.h"
#include "../../scene/nodes/aabb_node.h"

std::string getPrimitiveTopologyStr(VkPrimitiveTopology topology);
std::string getFormatStr(VkFormat format);
std::string getMemoryPropertyStr(VkMemoryPropertyFlags prop);
std::string getBufferUsageStr(VkBufferUsageFlags buff_usage);
std::string getMaterialTextures(uint32_t has_texture);
std::string getNodeFlagsStr(Scene::NodeTypeFlags node_type_flags);
std::string getSummaryForHierarchyStr(Scene::NodeIndex node_index, Scene::Hierarchy hierarchy_node, Scene::NodeTypeFlags node_type_flags, std::string node_name);
void printQuatImGUI(glm::quat q);
void printMatrixImGUI(const glm::mat4& matrix);
void editMatrixImGUI(const glm::mat4& matrix, std::function<void(glm::bvec3 tsr, glm::vec3 tr, glm::vec3 sc, glm::quat rot)> fn);
void printDecomposedMatrixImGUI(glm::mat4 matrix);
void printBoundingBoxImGUI(const BoundingBox& bb);
void printBoundingSphereImGUI(const BoundingSphere& bs);
void printBoundingFrustumImGUI(const BoundingFrustum& bf);
void printVulkanBufferImGUI(std::shared_ptr<VulkanBuffer> vk_buffer);
void printVertexBufferImGUI(std::shared_ptr<VertexBuffer> vtx, VkPrimitiveTopology topology);
void printMeshNodeImGUI(std::shared_ptr<MeshNode> pMesh);
void printCameraNodeImGUI(std::shared_ptr<CameraNode> pCamera);
void printAABBNodeImGUI(std::shared_ptr<AABBNode> pAABB);
void printHierarchyImGui(Scene::Hierarchy h);