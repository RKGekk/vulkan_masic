#include "scene_node.h"

SceneNode::SceneNode(std::shared_ptr<Scene> scene, Scene::NodeIndex node_index)
    : m_props(scene, m_props.m_node_index) {   
}

SceneNode::SceneNode(std::shared_ptr<Scene> scene, std::string name, Scene::NodeIndex parent)
    : m_props(scene, scene->addNode(name, parent)) {
}

SceneNode::SceneNode(std::shared_ptr<Scene> scene, std::string name, glm::mat4x4 to, Scene::NodeIndex parent)
    : m_props(scene, scene->addNode(to, name, parent)) {
}

SceneNode::~SceneNode() {}

bool SceneNode::VOnRestore() {
    return true;
}

bool SceneNode::VOnUpdate() {
    return true;
}

bool SceneNode::VOnLostDevice() {
    return true;
}

Scene::NodeIndex SceneNode::VGetChild() const {
    return m_props.m_scene->getNodeHierarchy(m_props.m_node_index).first_child;
}

Scene::NodeIndex SceneNode::VGetSibling() const {
    return m_props.m_scene->getNodeHierarchy(m_props.m_node_index).next_sibling;
}

Scene::NodeIndex SceneNode::VGetNodeIndex() const {
    return m_props.m_node_index;
}

Scene::NodeIndex SceneNode::GetParentIndex() const {
    return m_props.m_scene->getNodeHierarchy(m_props.m_node_index).parent;
}

const std::shared_ptr<Scene>& SceneNode::GetScene() {
    return m_props.m_scene;
};

const std::shared_ptr<SceneNode>& SceneNode::GetParent() {
    return m_props.m_scene->getProperty(m_props.m_node_index, Scene::NODE_TYPE_FLAG_NONE);
}

const SceneNodeProperties& SceneNode::Get() const {
    return m_props;
}

void SceneNode::SetTransform(const glm::mat4x4& to_parent) {
    m_props.m_scene->setNodeLocalTransform(m_props.m_node_index, to_parent);
}

void SceneNode::SetTranslation3(const glm::vec3& pos) {
    glm::mat4x4 local_transform = m_props.m_scene->getNodeLocalTransform(m_props.m_node_index);

	local_transform[3][0] = pos.x;
	local_transform[3][1] = pos.y;
	local_transform[3][2] = pos.z;

    m_props.m_scene->setNodeLocalTransform(m_props.m_node_index, local_transform);
}

void SceneNode::SetTranslation4(const glm::vec4& pos) {
    glm::mat4x4 local_transform = m_props.m_scene->getNodeLocalTransform(m_props.m_node_index);

	local_transform[3] = pos;

    m_props.m_scene->setNodeLocalTransform(m_props.m_node_index, local_transform);
}

void SceneNode::SetName(std::string name) {
    m_props.m_scene->setNodeName(m_props.m_node_index, name);
}

void SceneNode::SetNodeType(uint32_t node_type) {
    m_props.m_node_type = node_type;
}