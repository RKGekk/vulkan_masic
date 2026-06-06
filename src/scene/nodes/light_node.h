#pragma once

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

#include <memory>

#include "scene_node.h"
#include "light_node_properties.h"

class LightNode : public SceneNode {
public:
    enum class LightType {
	    DIRECTIONAL,
	    POINT,
	    SPOT
    };

	LightNode(std::shared_ptr<Scene> scene, Scene::NodeIndex node_index);
	LightNode(std::shared_ptr<Scene> scene, std::string name, Scene::NodeIndex parent = 0u);
	LightNode(std::shared_ptr<Scene> scene, std::string name, glm::mat4x4 transform, Scene::NodeIndex parent = 0u);
    LightNode(std::shared_ptr<Scene> scene, std::string name, glm::mat4x4 transform, const LightNodeProperties& light_props, LightType light_type, Scene::NodeIndex parent = 0u);

	virtual bool VOnRestore() override;
	virtual bool VOnUpdate() override;

    void setLightType(LightType light_type);
    LightType getLightType() const;

	void SetLightProperties(const LightNodeProperties& prop);
    LightNodeProperties GetLightProperties() const;

    void setStrength(glm::vec3 strength);
    glm::vec3 getStrength() const;

    void setFalloffStart(float falloff_start);
    float getFalloffStart() const;

    void setFalloffEnd(float falloff_end);
    float getFalloffEnd() const;

    void setSpotPower(float spot_power);
    float getSpotPower() const;

    void setOuterAngle(float outer_angle);
    float getOuterAngle() const;

    void setInnerAngle(float inner_angle);
    float getInnerAngle() const;

private:
    LightType m_light_type;
	glm::vec3 m_strength;
    float m_falloff_start;
    float m_falloff_end;
    float m_spot_power;
    float m_outer_angle;
    float m_inner_angle;
};