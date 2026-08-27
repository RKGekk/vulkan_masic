#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

#include "actor_component.h"
#include "../../scene/nodes/scene_node.h"
#include "../../scene/inverse_kinematics_solver.h"

#include <pugixml.hpp>

#include <memory>
#include <string>
#include <vector>

class InverseKinematicsComponent : public ActorComponent {
public:
    static const std::string g_name;

    InverseKinematicsComponent();
    InverseKinematicsComponent(const pugi::xml_node& data);
    virtual ~InverseKinematicsComponent();

    virtual bool VInit(const pugi::xml_node& data) override;
    virtual const std::string& VGetName() const override;
    virtual const ComponentDependecyList& VGetComponentDependecy() const override;
    virtual pugi::xml_node VGenerateXml() override;
    virtual void VPostInit() override;

private:
    bool Init(const pugi::xml_node& data);

    std::shared_ptr<InverseKinematicsSolver> m_iksolver;
};
