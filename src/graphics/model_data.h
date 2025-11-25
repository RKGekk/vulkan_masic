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

#include "../physics/bounding_box.h"
#include "../physics/bounding_sphere.h"
#include "vulkan_vertex_buffer.h"
#include "../scene/material.h"
#include "vertex_format.h"

#include <map>
#include <memory>
#include <string>

class ModelData {
public:
	ModelData();
	~ModelData() = default;

	void SetPrimitiveTopology(VkPrimitiveTopology primitive_toplogy);
	VkPrimitiveTopology GetPrimitiveTopology() const;

	void SetVertexBuffer(std::shared_ptr<VertexBuffer> vertex_buffer);
	const std::shared_ptr<VertexBuffer>& GetVertexBuffer() const;

	size_t GetIndexCount() const;
	size_t GetVertexCount() const;

	void SetMaterial(std::shared_ptr<Material> material);
	std::shared_ptr<Material> GetMaterial() const;

	void SetAABB(const BoundingBox& aabb);
	const BoundingBox& GetAABB() const;
    void SetSphere(const BoundingSphere& sphere) const;
	const BoundingSphere& GetSphere() const;

	const std::string& GetName() const;
	void SetName(std::string name);

	VkPipelineVertexInputStateCreateInfo GetVertextInputInfo();
	const VertexFormat& GetVertexFormat();
	void SetVertexFormat(const VertexFormat& format);

private:
	std::shared_ptr<VertexBuffer> m_vertex_buffer;
	VertexFormat m_vertex_format;
	std::shared_ptr<Material> m_material;

	VkPrimitiveTopology m_primitive_topology;
	BoundingBox m_AABB;
	BoundingSphere m_sphere;

	std::string m_name;
};