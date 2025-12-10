#pragma once

#include <cassert>
#include <iostream>
#include <memory>
#include <stdint.h>
#include <unordered_map>
#include <vector>

#define GLM_ENABLE_EXPERIMENTAL
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

class SceneNode;

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

	using NodeIndex = uint32_t;
    using NodeLevel = uint32_t;
	using NameIndex = uint32_t;
	using NodeType = uint32_t;
    using NodeTypeFlags = uint32_t;
    using NodeIndexArray = std::vector<NodeIndex>;
	using PropertyIndex = uint32_t;
	using Properties = std::unordered_map<NodeType, std::shared_ptr<SceneNode>>;

    static const NodeIndex NO_INDEX = -1;
	static const std::string NO_NAME;

	struct Hierarchy {
		NodeIndex parent = NO_INDEX;
		NodeIndex first_child = NO_INDEX;
		NodeIndex next_sibling = NO_INDEX;
		NodeLevel level = 0;
	};

    Scene();

	int addNode(NodeIndex parent_index = 0u);
	int addNode(std::string name, NodeIndex parent_index = 0);
	int addNode(const glm::mat4& local_transform, NodeIndex parent_index = 0);
	int addNode(const glm::mat4& local_transform, std::string name, NodeIndex parent_index = 0);

    void markAsChanged(NodeIndex node_index);
    int findNodeByName(const std::string& name) const;

    const std::string& getNodeName(NodeIndex node_index) const;
	void setNodeName(NodeIndex node_index, std::string name);

	const glm::mat4& getNodeLocalTransform(NodeIndex node_index) const;
	void setNodeLocalTransform(NodeIndex node_index, const glm::mat4& local_transform);

	const glm::mat4& getNodeGlobalTransform(NodeIndex node_index) const;
	const Hierarchy& getNodeHierarchy(NodeIndex node_index) const;
	NodeTypeFlags getNodeTypeFlags(NodeIndex node_index) const;

	std::shared_ptr<Properties> getProperties(NodeIndex node_index);
	std::shared_ptr<SceneNode> getProperty(NodeIndex node_index, NodeType node_type = NODE_TYPE_FLAG_NONE);
	void addProperty(std::shared_ptr<SceneNode> property, NodeIndex node_index = NO_INDEX);

    int getNodeLevel(NodeIndex node_index) const;
    bool recalculateGlobalTransforms();
    void deleteSceneNodes(const std::vector<NodeIndex>& nodes_indices_to_delete);
	void mergeScenes(const std::vector<Scene*>& scenes, const std::vector<glm::mat4>& root_transforms, const std::vector<uint32_t>& mesh_counts, bool merge_meshes, bool merge_materials);

private:
	NodeIndex findLastNonDeletedItem(const std::vector<NodeIndex>& new_indices, NodeIndex node);
	void shiftNodes(int startOffset, int nodeCount, int shiftAmount);

	std::vector<glm::mat4> m_local_transform;
	std::vector<glm::mat4> m_global_transform;
	std::vector<Hierarchy> m_hierarchy;
	std::vector<NodeIndexArray> m_dirty_at_level;

    std::unordered_map<NodeIndex, NodeTypeFlags> m_node_type_flags_map;
	std::unordered_map<NodeIndex, PropertyIndex> m_node_property_map;
	std::unordered_map<NodeIndex, NameIndex> m_node_name_map;

	std::vector<std::string> m_node_names;
	std::vector<std::shared_ptr<Properties>> m_properties;
};