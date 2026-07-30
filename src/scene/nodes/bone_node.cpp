#include "bone_node.h"

BoneNode::BoneNode(std::shared_ptr<Scene> scene, Scene::NodeIndex node_index) : SceneNode(std::move(scene), node_index), m_inverse_bind_matrice(glm::mat4(1.0f)) {}

BoneNode::BoneNode(std::shared_ptr<Scene> scene, std::string name, Scene::NodeIndex parent = 0u) : SceneNode(std::move(scene), std::move(name), parent), m_inverse_bind_matrice(glm::mat4(1.0f)) {}

BoneNode::BoneNode(std::shared_ptr<Scene> scene, std::string name, glm::mat4x4 transform, Scene::NodeIndex parent = 0u) : SceneNode(std::move(scene), std::move(name), transform, parent), m_inverse_bind_matrice(glm::mat4(1.0f)) {}

bool BoneNode::VOnRestore() {
    return SceneNode::VOnRestore();
}

bool BoneNode::VOnUpdate() {
    return SceneNode::VOnUpdate();
}

const glm::mat4x4& BoneNode::getInverseBindMatrice(const SkinName& skin_name) const {
    return m_bone_data.at(skin_name).inverse_bind_matrice;
}

const glm::mat4x4& BoneNode::getInverseBindMatrice() const {
    return (*m_bone_data.begin()).second.inverse_bind_matrice;
}

glm::mat4x4 BoneNode::getInverseBindMatriceT() const {
    return glm::transpose((*m_bone_data.begin()).second.inverse_bind_matrice);
}

glm::mat4x4 BoneNode::getInverseBindMatriceT(const SkinName& skin_name) const {
    return glm::transpose(m_bone_data.at(skin_name).inverse_bind_matrice);
}

void BoneNode::setInverseBindMatrice(const glm::mat4x4& inverse_bind_matrice) {
    (*m_bone_data.begin()).second.inverse_bind_matrice = inverse_bind_matrice;
}

void BoneNode::setInverseBindMatrice(const glm::mat4x4& inverse_bind_matrice, const SkinName& skin_name) {
    m_bone_data.at(skin_name).inverse_bind_matrice = inverse_bind_matrice;
}

BoneNode::JointIndex BoneNode::getJointIndex() const {
    return (*m_bone_data.begin()).second.joint_index;
}

BoneNode::JointIndex BoneNode::getJointIndex(const SkinName& skin_name) const {
    return m_bone_data.at(skin_name).joint_index;
}

void BoneNode::setJointIndex(JointIndex joint_idx) {
    (*m_bone_data.begin()).second.joint_index = joint_idx;
}

void BoneNode::setJointIndex(JointIndex joint_idx, const SkinName& skin_name) {
    m_bone_data.at(skin_name).joint_index = joint_idx;
}

const std::unordered_map<BoneNode::SkinName, BoneNode::BoneData>& BoneNode::getBoneDataMap() const {
    return m_bone_data;
}