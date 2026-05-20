#pragma once

#include <memory>

#include "scene_node.h"

#include <string>
#include <unordered_map>
#include <vector>

class ValueBagNode : public SceneNode {
public:
    using ValueName = std::string;
    struct ValuePosition {
        size_t size;
        size_t offset;
    };

	ValueBagNode(std::shared_ptr<Scene> scene, Scene::NodeIndex node_index);
	ValueBagNode(std::shared_ptr<Scene> scene, const std::string& name, const glm::mat4x4& transform, Scene::NodeIndex parent = 0u);

	virtual bool VOnRestore() override;
	virtual bool VOnUpdate() override;

	void AppendValue(const std::string& name, size_t size, const void* data);
    void InsertValue(const std::string& name, size_t size, size_t offset, const void* data);
    void SetValue(const std::string& name, const void* data);

    const void* GetValue(const std::string& name) const;
    const void* GetValue(size_t offset) const;
    const void* GetData() const;
    const std::unordered_map<ValueName, ValuePosition>& GetMetadata() const;

protected:
    std::unordered_map<ValueName, ValuePosition> m_metadata;
	std::vector<char> m_data;
};