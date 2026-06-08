#include "light_manager.h"

LightManager::LightManager() {}

void LightManager::CalcLighting() {
    for(const auto&[light_node, idx] : m_index_map) {
        m_lights[idx] = light_node->GetLightProperties();
    }
}

int LightManager::GetLightCount(std::shared_ptr<SceneNode> node) const {
    return m_lights.size();
}

const std::vector<LightNodeProperties>& LightManager::getLightsData() const {
    return m_lights;
}

void LightManager::AddLight(const std::shared_ptr<LightNode>& node) {
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

size_t LightManager::GetDirLightsCount() const {
    return m_dir_lights_size;
}

size_t LightManager::GetPointLightsCount() const {
    return m_point_lights_size;
}

size_t LightManager::GetSpotLightsCount() const {
    return m_spot_lights_size;
}