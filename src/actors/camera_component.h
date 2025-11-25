#pragma once

#include <filesystem>
#include <string>
#include <memory>
#include <map>
#include <vector>

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>
#include "glm/gtc/quaternion.hpp"
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <pugixml.hpp>

#include "actor_component.h"
#include "base_scene_node_component.h"

class CommandList;
class Material;
class BasicCameraNode;

class CameraComponent : public BaseSceneNodeComponent {
public:
	static const std::string g_name;
	static const std::vector<std::string> g_dependency_list;

	CameraComponent();
	CameraComponent(const pugi::xml_node& data);
	virtual ~CameraComponent();

	virtual bool VInit(const pugi::xml_node& data) override;
	virtual const std::string& VGetName() const override;
	virtual const ComponentDependecyList& VGetComponentDependecy() const override;
	virtual pugi::xml_node VGenerateXml() override;

	virtual const std::shared_ptr<BasicCameraNode>& VGetCameraNode();
	virtual const std::shared_ptr<SceneNode>& VGetSceneNode() override;

	virtual float GetFov();
	virtual void SetFov(float fov);

	virtual float GetNear();
	virtual void SetNear(float near_cut);

	virtual float GetFar();
	virtual void SetFar(float far_cut);

private:
    bool Init(const pugi::xml_node& data);

	std::shared_ptr<BasicCameraNode> m_camera_node;
};