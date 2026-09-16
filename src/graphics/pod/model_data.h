#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>
#include "glm/gtc/quaternion.hpp"
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../../physics/bounding_box.h"
#include "../../physics/bounding_sphere.h"
#include "material.h"
#include "shader_signature.h"

#include <map>
#include <memory>
#include <string>
#include <unordered_map>

class VulkanBuffer;

class ModelData {
public:
	ModelData();
	~ModelData() = default;

	void SetPrimitiveTopology(VkPrimitiveTopology primitive_toplogy);
	VkPrimitiveTopology GetPrimitiveTopology() const;

	void SetVertexBuffer(std::shared_ptr<VulkanBuffer> vertex_buffer, VertexFormat::BindingNum binding);
	const std::shared_ptr<VulkanBuffer>& GetVertexBuffer(VertexFormat::BindingNum binding) const;

	void SetIndexBuffer(std::shared_ptr<VulkanBuffer> index_buffer);
	const std::shared_ptr<VulkanBuffer>& GetIndexBuffer() const;

	size_t GetIndexCount() const;
	size_t GetInstanceCount() const;
	size_t GetVertexCount() const;

	void SetMaterial(std::shared_ptr<Material> material);
	std::shared_ptr<Material> GetMaterial() const;

	void SetAABB(const BoundingBox& aabb);
	const BoundingBox& GetAABB() const;
    void SetSphere(const BoundingSphere& sphere);
	const BoundingSphere& GetSphere() const;

	const std::string& GetName() const;
	void SetName(std::string name);

	const std::shared_ptr<ShaderSignature>& GetShaderSignature() const;
	void SetShaderSignature(std::shared_ptr<ShaderSignature> format);

private:
	std::unordered_map<VertexFormat::BindingNum, std::shared_ptr<VulkanBuffer>> m_vertex_buffers;
	std::shared_ptr<VulkanBuffer> m_index_buffer;
	
	std::shared_ptr<ShaderSignature> m_shader_signature;
	std::shared_ptr<Material> m_material;

	VkPrimitiveTopology m_primitive_topology;
	BoundingBox m_AABB;
	BoundingSphere m_sphere;

	std::string m_name;
};