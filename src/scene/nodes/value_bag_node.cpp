#include "value_bag_node.h"

ValueBagNode::ValueBagNode(std::shared_ptr<Scene> scene, Scene::NodeIndex node_index) : SceneNode(std::move(scene), node_index) {
    SetNodeType(Scene::NODE_TYPE_FLAG_VALUE_BAG);
}

ValueBagNode::ValueBagNode(std::shared_ptr<Scene> scene, const std::string& name, const glm::mat4x4& transform, Scene::NodeIndex parent) : SceneNode(std::move(scene), std::move(name), transform, parent) {
    SetNodeType(Scene::NODE_TYPE_FLAG_VALUE_BAG);
}

bool ValueBagNode::VOnRestore() {
    return true;
}

bool ValueBagNode::VOnUpdate() {
    return SceneNode::VOnUpdate();
}

void ValueBagNode::AppendValue(const std::string& name, size_t size, const void* data) {
    size_t start_offset = m_data.size();
    size_t sz = m_data.size() + size;
    m_data.resize(sz);
    memcpy(m_data.data() + start_offset, data, size);

    m_metadata[name] = {size, start_offset};
}
    
void ValueBagNode::InsertValue(const std::string& name, size_t size, size_t offset, const void* data) {
    memcpy(m_data.data() + offset, data, size);
}

void ValueBagNode::SetValue(const std::string& name, const void* data) {
    const ValuePosition& vp = m_metadata.at(name);
    memcpy(
        m_data.data() + vp.offset,
        data,
        vp.size
    );
}

bool ValueBagNode::HasName(const ValueName& name) const {
    return m_metadata.contains(name);
}

const void* ValueBagNode::GetValue(const ValueBagNode::ValueName& name) const {
    return m_data.data() + m_metadata.at(name).offset;
}

const void* ValueBagNode::GetValue(size_t offset) const {
    return m_data.data() + offset;
}

const void* ValueBagNode::GetData() const {
    return m_data.data();
}

const std::unordered_map<ValueBagNode::ValueName, ValueBagNode::ValuePosition>& ValueBagNode::GetMetadata() const {
    return m_metadata;
}