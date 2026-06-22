#pragma once

#include "nodes/light_node.h"
#include "nodes/camera_node.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class LightManager {
public:
    LightManager();

    void CalcLighting(const std::shared_ptr<CameraNode>& camera_node);
    int GetLightCount(const std::shared_ptr<SceneNode>& node) const;
    const std::vector<LightNodeProperties>& getLightsData(const std::shared_ptr<SceneNode>& node) const;
    const std::vector<LightNodeProperties>& getAllLightsData() const;

    void AddLight(const std::shared_ptr<LightNode>& node);
	void RemoveLight(const std::shared_ptr<LightNode>& node);

    void DecorateValueBag(std::shared_ptr<SceneNode>& node) const;

	size_t GetDirLightsCount() const;
	size_t GetPointLightsCount() const;
	size_t GetSpotLightsCount() const;

    static const std::string& getLightBufferName();
    static const std::string& getLightResourceCfgName();

private:
    static const std::string m_light_buffer_name;
    static const std::string m_light_resource_cfg_name;
    std::vector<LightNodeProperties> m_lights;
    uint32_t m_dir_lights_size;
    uint32_t m_point_lights_size;
    uint32_t m_spot_lights_size;
	std::unordered_map<std::shared_ptr<LightNode>, size_t> m_index_map;
};