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
#include "../scene/nodes/scene_node.h"

#include <pugixml.hpp>

#include <memory>
#include <string>
#include <vector>

class TransformComponent : public ActorComponent {
public:
    static const std::string g_name;

    TransformComponent();
    TransformComponent(const pugi::xml_node& data);
    virtual ~TransformComponent();

    virtual bool VInit(const pugi::xml_node& data) override;
    virtual const std::string& VGetName() const override;
    virtual const ComponentDependecyList& VGetComponentDependecy() const override;
    virtual pugi::xml_node VGenerateXml() override;
    virtual void VPostInit() override;

    const glm::mat4x4& GetTransform() const;
    glm::mat4x4 GetTransformT() const;
    glm::mat4x4 GetInvTransform() const;
    glm::mat4x4 GetInvTransformT() const;
    void Decompose(glm::vec3& pos, glm::vec3& rot, glm::vec3& scale) const;
    void Decompose(glm::vec3& pos, glm::quat& rot, glm::vec3& scale) const;

    void SetTransform(const glm::mat4x4& newTransform);
    void SetTransform(const glm::vec3& pos, const glm::vec3& rot, glm::vec3& scale);
    void SetTransform(const glm::vec3& pos, const glm::quat& rot, glm::vec3& scale);

    glm::vec3 GetTranslation3f() const;
    const glm::vec4& GetTranslation4f() const;

    void SetTranslation3f(const glm::vec3& pos);
    void SetTranslation4f(const glm::vec4& pos);
    void SetTranslation4x4f(const glm::mat4x4& pos);

    glm::vec3 GetLookAt() const;
    glm::vec3 GetLookRight() const;
    glm::vec3 GetLookUp() const;

    glm::vec3 GetYawPitchRoll() const;

    glm::vec3 GetForward3f() const;
    const glm::vec4& GetForward4f() const;

    glm::vec3 GetUp3f() const;
    const glm::vec4& GetUp4f() const;

    glm::vec3 GetRight3f() const;
    const glm::vec4& GetRight4f() const;

    std::shared_ptr<SceneNode> GetSceneNode();
    Scene::NodeIndex GetSceneNodeIndex();



private:
    bool Init(const pugi::xml_node& data);

    std::shared_ptr<SceneNode> m_scene_node;

    glm::vec4 m_forward;
    glm::vec4 m_up;
    glm::vec4 m_right;

    const glm::vec4 DEFAULT_FORWARD_VECTOR = glm::vec4( 0.0f, 0.0f, 1.0f, 0.0f );
    const glm::vec4 DEFAULT_UP_VECTOR = glm::vec4( 0.0f, 1.0f, 0.0f, 0.0f );
    const glm::vec4 DEFAULT_RIGHT_VECTOR = glm::vec4( 1.0f, 0.0f, 0.0f, 0.0f );
    const float EPSILON = 0.001f;
};
