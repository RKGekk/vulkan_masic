#include "scene_node.h"

SceneNode::SceneNode(std::shared_ptr<Scene> scene, Scene::NodeIndex node_index)
    : m_scene(scene)
    , m_props(m_scene, m_node_index) {

    m_scene->addProperty(m_node_index, shared_from_this());
}

SceneNode::SceneNode(std::shared_ptr<Scene> scene, std::string name, Scene::NodeIndex parent)
    : m_scene(scene)
    , m_node_index(m_scene->addNode(name, parent))
    , m_props(m_scene, m_node_index) {

    m_scene->addProperty(m_node_index, shared_from_this());
}

SceneNode::SceneNode(std::shared_ptr<Scene> scene, std::string name, glm::mat4x4 to, Scene::NodeIndex parent)
    : m_scene(scene)
    , m_node_index(m_scene->addNode(to, name, parent))
    , m_props(m_scene, m_node_index) {

    m_scene->addProperty(m_node_index, shared_from_this());
}

SceneNode::~SceneNode() {}

bool SceneNode::VOnRestore() {}

bool SceneNode::VOnUpdate() {}

bool SceneNode::VOnLostDevice() {}

Scene::NodeIndex SceneNode::VGetChild() const {
    return m_scene->getNodeHierarchy(m_node_index).first_child;
}

Scene::NodeIndex SceneNode::VGetSibling() const {
    return m_scene->getNodeHierarchy(m_node_index).next_sibling;
}

Scene::NodeIndex SceneNode::VGetNodeIndex() const {
    return m_node_index;
}

Scene::NodeIndex SceneNode::GetParentIndex() const {
    return m_scene->getNodeHierarchy(m_node_index).parent;
}

const SceneNodeProperties& SceneNode::Get() const {
    return m_props;
}

void SceneNode::SetTransform(const glm::mat4x4& to_parent) {
    m_scene->setNodeLocalTransform(m_node_index, to_parent);
}

void SceneNode::SetTranslation3(const glm::vec3& pos) {
    glm::mat4x4 local_transform = m_scene->getNodeLocalTransform(m_node_index);

	local_transform[3][0] = pos.x;
	local_transform[3][1] = pos.y;
	local_transform[3][2] = pos.z;

    m_scene->setNodeLocalTransform(m_node_index, local_transform);
}

void SceneNode::SetTranslation4(const glm::vec4& pos) {
    glm::mat4x4 local_transform = m_scene->getNodeLocalTransform(m_node_index);

	local_transform[3] = pos;

    m_scene->setNodeLocalTransform(m_node_index, local_transform);
}

void SceneNode::SetName(std::string name) {
    m_scene->setNodeName(m_node_index, name);
}

void SceneNode::SetNodeType(uint32_t node_type) {
    m_props.m_node_type = node_type;
}