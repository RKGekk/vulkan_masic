#pragma once

#include <cassert>
#include <iostream>
#include <memory>
#include <stdint.h>
#include <unordered_map>
#include <vector>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext.hpp>

#include "material.h"

constexpr const int MAX_NODE_LEVEL = 32;

class Scene {

public:
    static const uint32_t NODE_TYPE_FLAG_NONE = 0u;
    static const uint32_t NODE_TYPE_FLAG_MESH = 1u;
	static const uint32_t NODE_TYPE_FLAG_LIGHT = 2u;
	static const uint32_t NODE_TYPE_FLAG_CAMERA = 4u;
	static const uint32_t NODE_TYPE_FLAG_SHADOW_CAMERA = 8u;
	static const uint32_t NODE_TYPE_FLAG_AABB = 16u;
	static const uint32_t NODE_TYPE_FLAG_SPHERE = 32u;
	static const uint32_t NODE_TYPE_FLAG_BONE = 64u;

	using MaterialIndex = uint32_t;
	using MeshIndex = uint32_t;
	using NodeIndex = uint32_t;
    using NodeLevel = uint32_t;
	using NameIndex = uint32_t;
	using GroupIndex = uint32_t;
    using NodeTypeFlags = uint32_t;
    using NodeIndexArray = std::vector<NodeIndex>;

    static const NodeIndex NO_INDEX = -1;

	struct Hierarchy {
		NodeIndex parent = NO_INDEX;
		NodeIndex first_child = NO_INDEX;
		NodeIndex next_sibling = NO_INDEX;
		NodeLevel level = 0;
	};

    Scene();

	int addNode(NodeIndex parent_index = 0u);
    void markAsChanged(NodeIndex node_index);
    int findNodeByName(const std::string& name);
    const std::string& getNodeName(NodeIndex node_index);
    void setNodeName(NodeIndex node_index, const std::string& name);
    int getNodeLevel(NodeIndex node_index);
    bool recalculateGlobalTransforms();
    void deleteSceneNodes(const std::vector<NodeIndex>& nodes_indices_to_delete);

private:
	NodeIndex findLastNonDeletedItem(const std::vector<NodeIndex>& newIndices, NodeIndex node);

	std::vector<glm::mat4> m_local_transform;
	std::vector<glm::mat4> m_global_transform;
	std::vector<Hierarchy> m_hierarchy;
	std::vector<NodeIndexArray> m_dirty_at_level;

    std::unordered_map<NodeIndex, NodeTypeFlags> m_node_type_flags_map;
	std::unordered_map<NodeIndex, MeshIndex> m_node_mesh_map;
	std::unordered_map<NodeIndex, GroupIndex> m_node_group_map;
	std::unordered_map<NodeIndex, MaterialIndex> m_node_material_map;
	std::unordered_map<NodeIndex, NameIndex> m_node_name_map;

	std::vector<std::string> m_node_names;
	std::vector<Material> m_materials;
};