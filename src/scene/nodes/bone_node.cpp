#include "bone_node.h"

BoneNode::BoneNode(std::shared_ptr<Scene> scene, Scene::NodeIndex node_index) : SceneNode(std::move(scene), node_index), m_inverse_bind_matrice(glm::mat4(1.0f)) {}

BoneNode::BoneNode(std::shared_ptr<Scene> scene, std::string name, Scene::NodeIndex parent = 0u) : SceneNode(std::move(scene), std::move(name), parent), m_inverse_bind_matrice(glm::mat4(1.0f)) {}

BoneNode::BoneNode(std::shared_ptr<Scene> scene, std::string name, glm::mat4x4 transform, Scene::NodeIndex parent = 0u) : SceneNode(std::move(scene), std::move(name), transform, parent), m_inverse_bind_matrice(glm::mat4(1.0f)) {}

BoneNode::BoneNode(std::shared_ptr<Scene> scene, std::string name, glm::mat4x4 transform, glm::mat4 inverse_bind_matrice, Scene::NodeIndex parent = 0u) : SceneNode(std::move(scene), std::move(name), transform, parent), m_inverse_bind_matrice(inverse_bind_matrice) {}

bool BoneNode::VOnRestore() {
    return SceneNode::VOnRestore();
}

bool BoneNode::VOnUpdate() {
    return SceneNode::VOnUpdate();
}

const glm::mat4x4& BoneNode::getInverseBindMatrice() const {
    return m_inverse_bind_matrice;
}

glm::mat4x4 BoneNode::getInverseBindMatriceT() const {
    return glm::transpose(m_inverse_bind_matrice);
}

void BoneNode::setInverseBindMatrice(const glm::mat4x4& inverse_bind_matrice) {
    m_inverse_bind_matrice = inverse_bind_matrice;
}