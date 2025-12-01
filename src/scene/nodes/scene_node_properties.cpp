#include "scene_node_properties.h"

SceneNodeProperties::SceneNodeProperties(std::shared_ptr<Scene> scene, Scene::NodeIndex node_index, Scene::NodeType node_type) : m_scene(std::move(scene)), m_node_index(node_index), m_node_type(node_type) {}

const glm::mat4x4& SceneNodeProperties::ToParent() const {
    return m_scene->getNodeLocalTransform(m_node_index);
}

glm::mat4x4 SceneNodeProperties::ToParentT() const {
    return glm::transpose(m_scene->getNodeLocalTransform(m_node_index));
}

const glm::mat4x4& SceneNodeProperties::ToRoot() const {
    return m_scene->getNodeGlobalTransform(m_node_index);
}

glm::mat4x4 SceneNodeProperties::ToRootT() const {
    return glm::transpose(m_scene->getNodeGlobalTransform(m_node_index));
}

const glm::vec4& SceneNodeProperties::ToParentTranslation4() const {
    return m_scene->getNodeLocalTransform(m_node_index)[3];
}

glm::vec3 SceneNodeProperties::ToParentTranslation3() const {
    return glm::vec3(m_scene->getNodeLocalTransform(m_node_index)[3]);
}

const glm::vec4& SceneNodeProperties::ToRootTranslation4() const {
    return m_scene->getNodeGlobalTransform(m_node_index)[3];
}

glm::vec3 SceneNodeProperties::ToRootTranslation3() const{
    return glm::vec3(m_scene->getNodeGlobalTransform(m_node_index)[3]);
}

glm::vec3 SceneNodeProperties::ToParentDirection() const {
    glm::mat4x4 just_rot = m_scene->getNodeLocalTransform(m_node_index);
	just_rot[3][0] = 0.0f;
	just_rot[3][1] = 0.0f;
	just_rot[3][2] = 0.0f;
	just_rot[3][3] = 1.0f;

	glm::vec4 forward(0.0f, 0.0f, 1.0f, 0.0f);

	return glm::vec3(just_rot * forward);
}

glm::vec3 SceneNodeProperties::ToParentUp() const {
    glm::mat4x4 just_rot = m_scene->getNodeLocalTransform(m_node_index);
	just_rot[3][0] = 0.0f;
	just_rot[3][1] = 0.0f;
	just_rot[3][2] = 0.0f;
	just_rot[3][3] = 1.0f;

	glm::vec4 up(0.0f, -1.0f, 0.0f, 0.0f);
    
	return glm::vec3(just_rot * up);
}

glm::vec3 SceneNodeProperties::ToRootDirection() const {
    glm::mat4x4 just_rot = m_scene->getNodeGlobalTransform(m_node_index);
	just_rot[3][0] = 0.0f;
	just_rot[3][1] = 0.0f;
	just_rot[3][2] = 0.0f;
	just_rot[3][3] = 1.0f;

	glm::vec4 forward(0.0f, 0.0f, 1.0f, 0.0f);

	return glm::vec3(just_rot * forward);
}

glm::vec3 SceneNodeProperties::ToRootUp() const {
    glm::mat4x4 just_rot = m_scene->getNodeGlobalTransform(m_node_index);
	just_rot[3][0] = 0.0f;
	just_rot[3][1] = 0.0f;
	just_rot[3][2] = 0.0f;
	just_rot[3][3] = 1.0f;

	glm::vec4 up(0.0f, -1.0f, 0.0f, 0.0f);
    
	return glm::vec3(just_rot * up);
}

glm::mat4x4 SceneNodeProperties::FromParent() const {
    return glm::inverse(m_scene->getNodeLocalTransform(m_node_index));
}

glm::mat4x4 SceneNodeProperties::FromParentT() const {
    return glm::transpose(glm::inverse(m_scene->getNodeLocalTransform(m_node_index)));
}

glm::mat4x4 SceneNodeProperties::FromRoot() const {
    return glm::inverse(m_scene->getNodeGlobalTransform(m_node_index));
}

glm::mat4x4 SceneNodeProperties::FromRootT() const {
    return glm::transpose(glm::inverse(m_scene->getNodeGlobalTransform(m_node_index)));
}

const char* SceneNodeProperties::NameCstr() const {
    return m_scene->getNodeName(m_node_index).c_str();
}

const std::string& SceneNodeProperties::Name() const {
    return m_scene->getNodeName(m_node_index);
}

uint32_t SceneNodeProperties::GetNodeType() const {
    return m_node_type;
}