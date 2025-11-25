#include "mesh_node.h"
#include "aabb_node.h"

MeshNode::MeshNode(std::shared_ptr<Scene> scene, Scene::NodeIndex node_index) : SceneNode(std::move(scene), node_index) {}

MeshNode::MeshNode(std::shared_ptr<Scene> scene, const std::string& name, const glm::mat4x4& transform, Scene::NodeIndex parent)
    : SceneNode(std::move(scene), std::move(name), transform, parent) {}

MeshNode::MeshNode(std::shared_ptr<Scene> scene, const std::string& name, const glm::mat4x4& transform, const MeshList& meshes, Scene::NodeIndex parent) : SceneNode(std::move(scene), std::move(name), transform, parent) {
    for (const auto& mesh : meshes) {
        AddMesh(mesh);
    }
}

MeshNode::MeshNode(std::shared_ptr<Scene> scene, const std::string& name, const MeshList& meshes, Scene::NodeIndex parent) : SceneNode(std::move(scene), std::move(name), parent) {
    for (const auto& mesh : meshes) {
        AddMesh(mesh);
    }
}

bool MeshNode::VOnRestore() {
    return true;
}

bool MeshNode::VOnUpdate() {
    return SceneNode::VOnUpdate();
}

bool MeshNode::AddMesh(const std::shared_ptr<ModelData>& mesh) {
    if (!mesh) return false;

    MeshList::const_iterator iter = std::find(m_meshes.begin(), m_meshes.end(), mesh);
    if (iter != m_meshes.cend()) return false;

    m_meshes.push_back(mesh);
    CalcAABB();

    return true;
}

void MeshNode::RemoveMesh(const std::shared_ptr<ModelData>& mesh) {
    if (!mesh) return;

    MeshList::const_iterator iter = std::find(m_meshes.begin(), m_meshes.end(), mesh);
    if (iter == m_meshes.end()) return;

    m_meshes.erase(iter);

    CalcAABB();
}

const MeshNode::MeshList& MeshNode::GetMeshes() const {
    return m_meshes;
}

const std::shared_ptr<ModelData>& MeshNode::GetMesh(size_t index) const {
    return m_meshes.at(index);
}

void MeshNode::CalcAABB() {
    BoundingBox aabb_max;
    int sz = m_meshes.size();
    for (int i = 0u; i < sz; ++i) {
        const auto& mesh = m_meshes[i];
        BoundingBox aabb_original = mesh->GetAABB();
        if (i == 0) aabb_max = aabb_original;
        else BoundingBox::CreateMerged(aabb_max, aabb_max, aabb_original);
    }
    std::shared_ptr<AABBNode> aabb = std::make_shared<AABBNode>(m_scene, m_node_index);
    aabb->setAABB(aabb_max);
    m_scene->addProperty(m_node_index, std::move(aabb));
}