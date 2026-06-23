#include "light_manager.h"
#include "nodes/value_bag_node.h"

LightManager::LightManager() : m_dir_lights_size(0u), m_point_lights_size(0u), m_spot_lights_size(0u) {}

const std::string LightManager::m_light_buffer_name = "light_ubo";
const std::string LightManager::m_light_resource_cfg_name = "light_uniform_resource";

void LightManager::CalcLighting(const std::shared_ptr<CameraNode>& camera_node) {
    for(const auto&[light_node, idx] : m_index_map) {
        m_lights[idx] = light_node->GetLightProperties();
    }
}

int LightManager::GetLightCount(const std::shared_ptr<SceneNode>& node) const {
    return m_lights.size();
}

const std::vector<LightNodeProperties>& LightManager::getLightsData(const std::shared_ptr<SceneNode>& node) const {
    return m_lights;
}

const std::vector<LightNodeProperties>& LightManager::getAllLightsData() const {
    return m_lights;
}

void LightManager::AddLight(const std::shared_ptr<LightNode>& node) {
    using namespace std::literals;

    switch (node->getLightType()) {
        case LightNode::LightType::DIRECTIONAL : {
                size_t light_id = m_dir_lights_size;
                m_lights.insert(m_lights.begin(), node->GetLightProperties());
                ++m_dir_lights_size;
                m_index_map[node] = light_id;
            }
            break;

        case LightNode::LightType::POINT : {
                size_t light_id = m_dir_lights_size + m_point_lights_size;
                m_lights.insert(m_lights.begin() + light_id, node->GetLightProperties());
                ++m_point_lights_size;
                m_index_map[node] = light_id;
            }
            break;

        case LightNode::LightType::SPOT : {
                size_t light_id = m_dir_lights_size + m_point_lights_size + m_spot_lights_size;
                m_lights.insert(m_lights.begin() + light_id, node->GetLightProperties());
                ++m_spot_lights_size;
                m_index_map[node] = light_id;
            }
            break;
    
        default:
            break;
    }
}

void LightManager::RemoveLight(const std::shared_ptr<LightNode>& node) {
    size_t light_id = m_index_map.at(node);
    m_lights.erase(m_lights.begin() + light_id);
    m_index_map.erase(node);

    switch (node->getLightType()) {
        case LightNode::LightType::DIRECTIONAL : --m_dir_lights_size; break;
        case LightNode::LightType::POINT : --m_point_lights_size; break;
        case LightNode::LightType::SPOT : --m_spot_lights_size; break;
        default: break;
    }
}

void LightManager::DecorateValueBag(const std::shared_ptr<SceneNode>& node) const {
    using namespace std::literals;
    std::shared_ptr<Scene> scene = node->GetScene();
    std::shared_ptr<ValueBagNode> value_bag_node = std::dynamic_pointer_cast<ValueBagNode>(scene->getProperty(node->VGetNodeIndex(), Scene::NODE_TYPE_FLAG_VALUE_BAG));
    if(!value_bag_node) {
        value_bag_node = std::make_shared<ValueBagNode>(scene, node->VGetNodeIndex());
        scene->addProperty(value_bag_node);
    }

    if(value_bag_node->HasName("u_num_dir_lights"s)) {
        value_bag_node->SetValue("u_num_dir_lights"s, &m_dir_lights_size);
    }
    else {
        value_bag_node->AppendValue("u_num_dir_lights"s, sizeof(uint32_t), &m_dir_lights_size);
    }

    if(value_bag_node->HasName("u_num_point_lights"s)) {
        value_bag_node->SetValue("u_num_point_lights"s, &m_point_lights_size);
    }
    else {
        value_bag_node->AppendValue("u_num_point_lights"s, sizeof(uint32_t), &m_point_lights_size);
    }
    
    if(value_bag_node->HasName("u_num_spot_lights"s)) {
        value_bag_node->SetValue("u_num_spot_lights"s, &m_spot_lights_size);
    }
    else {
        value_bag_node->AppendValue("u_num_spot_lights"s, sizeof(uint32_t), &m_spot_lights_size);
    }
}

size_t LightManager::GetDirLightsCount() const {
    return m_dir_lights_size;
}

size_t LightManager::GetPointLightsCount() const {
    return m_point_lights_size;
}

size_t LightManager::GetSpotLightsCount() const {
    return m_spot_lights_size;
}

const std::string& LightManager::getLightBufferName() {
    using namespace std::literals;
    return m_light_buffer_name;
}

const std::string& LightManager::getLightResourceCfgName() {
    using namespace std::literals;
    return m_light_resource_cfg_name;
}