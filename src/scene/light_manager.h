#pragma once

#include "nodes/light_node.h"

#include <memory>
#include <unordered_map>
#include <vector>

class LightManager {
public:
    LightManager();

    void CalcLighting();
    int GetLightCount(std::shared_ptr<SceneNode> node) const;
    const std::vector<LightNodeProperties>& getLightsData() const;

    void AddLight(const std::shared_ptr<LightNode>& node);
	void RemoveLight(const std::shared_ptr<LightNode>& node);

	size_t GetDirLightsCount() const;
	size_t GetPointLightsCount() const;
	size_t GetSpotLightsCount() const;

private:
    std::vector<LightNodeProperties> m_lights;
    int m_dir_lights_size;
    int m_point_lights_size;
    int m_spot_lights_size;
	std::unordered_map<std::shared_ptr<LightNode>, size_t> m_index_map;
};