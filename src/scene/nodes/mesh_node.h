#pragma once

#include <memory>

#include <DirectXMath.h>
#include <DirectXCollision.h>

#include "scene_node.h"
#include "../../graphics/model_data.h"

class MeshNode : public SceneNode {
public:
	using MeshList = std::vector<std::shared_ptr<ModelData>>;

	MeshNode(std::shared_ptr<Scene> scene, Scene::NodeIndex node_index);
	MeshNode(std::shared_ptr<Scene> scene, const std::string& name, const glm::mat4x4& transform, Scene::NodeIndex parent = 0u);
	MeshNode(std::shared_ptr<Scene> scene, const std::string& name, const glm::mat4x4& transform, const MeshList& meshes, Scene::NodeIndex parent = 0u);
	MeshNode(std::shared_ptr<Scene> scene, const std::string& name, const MeshList& meshes, Scene::NodeIndex parent = 0u);

	virtual bool VOnRestore() override;
	virtual bool VOnUpdate() override;

	bool AddMesh(const std::shared_ptr<ModelData>& mesh);
	void RemoveMesh(const std::shared_ptr<ModelData>& mesh);
	const MeshList& GetMeshes() const;
	const std::shared_ptr<ModelData>& GetMesh(size_t index = 0) const;

protected:
	void CalcAABB();

	MeshList m_meshes;
};