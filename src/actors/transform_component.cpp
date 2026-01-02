#include "transform_component.h"

#include "../tools/string_tools.h"
#include "../application.h"
#include "../engine/views/human_view.h"
#include "../engine/base_engine_logic.h"
#include "../events/cicadas/evt_data_destroy_scene_component.h"

const std::string TransformComponent::g_name = "TransformComponent";

const glm::vec4 TransformComponent::DEFAULT_FORWARD_VECTOR = glm::vec4( 0.0f, 0.0f, -1.0f, 0.0f );
const glm::vec4 TransformComponent::DEFAULT_UP_VECTOR = glm::vec4( 0.0f, 1.0f, 0.0f, 0.0f );
const glm::vec4 TransformComponent::DEFAULT_RIGHT_VECTOR = glm::vec4( 1.0f, 0.0f, 0.0f, 0.0f );
    
const float TransformComponent::EPSILON = 0.001f;

TransformComponent::TransformComponent() {
    m_forward = DEFAULT_FORWARD_VECTOR;
    m_up = DEFAULT_UP_VECTOR;
    m_right = DEFAULT_RIGHT_VECTOR;

    std::shared_ptr<Scene> scene_ptr = Application::Get().GetGameLogic()->GetHumanView()->VGetScene();
    m_scene_node = std::make_shared<SceneNode>(scene_ptr, g_name, glm::mat4(1.0f));
    scene_ptr->addProperty(m_scene_node);
}

TransformComponent::TransformComponent(const pugi::xml_node& data) {
    m_forward = DEFAULT_FORWARD_VECTOR;
    m_up = DEFAULT_UP_VECTOR;
    m_right = DEFAULT_RIGHT_VECTOR;

    std::shared_ptr<Scene> scene_ptr = Application::Get().GetGameLogic()->GetHumanView()->VGetScene();
    m_scene_node = std::make_shared<SceneNode>(scene_ptr, g_name, glm::mat4(1.0f));
    scene_ptr->addProperty(m_scene_node);

    Init(data);
}

TransformComponent::~TransformComponent() {
    std::shared_ptr<Actor> act = GetOwner();
	std::shared_ptr<EvtData_Destroy_Scene_Component> pDestroyActorComponentEvent = std::make_shared<EvtData_Destroy_Scene_Component>(act->GetId(), VGetId(), m_scene_node);
	IEventManager::Get()->VQueueEvent(pDestroyActorComponentEvent);
}

const std::string& TransformComponent::VGetName() const {
    return g_name;
}

const ComponentDependecyList& TransformComponent::VGetComponentDependecy() const {
    static const ComponentDependecyList component_dep = {};
    return component_dep;
}

pugi::xml_node TransformComponent::VGenerateXml() {
    return pugi::xml_node();
}

void TransformComponent::VPostInit() {
    std::shared_ptr<Actor> act = GetOwner();
    std::string name = act->GetName();
    m_scene_node->SetName(name + VGetName());
}

const glm::mat4x4& TransformComponent::GetTransform() const {
    return m_scene_node->Get().ToParent();
}

glm::mat4x4 TransformComponent::GetTransformT() const {
    return m_scene_node->Get().ToParentT();
}

glm::mat4x4 TransformComponent::GetInvTransform() const {
    return m_scene_node->Get().FromParent();
}

glm::mat4x4 TransformComponent::GetInvTransformT() const {
    return m_scene_node->Get().FromParentT();
}

void TransformComponent::Decompose(glm::vec3& pos, glm::vec3& ypr, glm::vec3& scale) const {
    glm::mat4x4 mat = m_scene_node->Get().ToParent();

    pos = mat[3];

    for (int i = 0; i < 3; ++i) {
        scale[i] = glm::length(mat[i]);
        mat[i] /= scale[i];
    }

    glm::vec3 res = glm::eulerAngles(glm::toQuat(mat));
    ypr = glm::vec3(res.y, res.x, res.z);
}

void TransformComponent::Decompose(glm::vec3& pos, glm::quat& rot, glm::vec3& scale) const {
    glm::mat4x4 mat = m_scene_node->Get().ToParent();

    pos = mat[3];

    for (int i = 0; i < 3; ++i) {
        scale[i] = glm::length(mat[i]);
        mat[i] /= scale[i];
    }

    rot = glm::toQuat(mat);
}

void TransformComponent::SetTransform(const glm::mat4x4& newTransform) {
    m_scene_node->SetTransform(newTransform);
}

void TransformComponent::SetTransform(const glm::vec3& pos, const glm::vec3& ypr, glm::vec3& scale) {
    glm::mat4x4 S = glm::scale(scale);
	glm::mat4x4 R = glm::eulerAngleYXZ(ypr.x, ypr.y, ypr.z);
	glm::mat4x4 T = glm::translate(pos);
    SetTransform(T * R * S);
}

void TransformComponent::SetTransform(const glm::vec3& pos, const glm::quat& rot, glm::vec3& scale) {
    glm::mat4x4 S(glm::scale(scale));
	glm::mat4x4 R(rot);
	glm::mat4x4 T(glm::translate(pos));
	SetTransform(T * R * S);
}

glm::vec3 TransformComponent::GetTranslation3f() const {
    return m_scene_node->Get().ToParentTranslation3();
}

const glm::vec4& TransformComponent::GetTranslation4f() const {
    return m_scene_node->Get().ToParentTranslation4();
}

void TransformComponent::SetTranslation3f(const glm::vec3& pos) {
    m_scene_node->SetTranslation3(pos);
}

void TransformComponent::SetTranslation4f(const glm::vec4& pos) {
    m_scene_node->SetTranslation4(pos);
}

void TransformComponent::SetTranslation4x4f(const glm::mat4x4& pos) {
    m_scene_node->SetTranslation4(pos[3]);
}

void TransformComponent::SetRotation(const glm::quat& rot) {
    m_scene_node->SetRotation(rot);
}

glm::vec3 TransformComponent::GetLookAt() const {
    //glm::quat justRot = glm::normalize(glm::quat(m_scene_node->Get().ToParent()));
    glm::quat justRot = glm::normalize(glm::quat(m_scene_node->Get().FromParent()));
    glm::vec3 out = m_forward * justRot;
    //glm::vec3 out = glm::rotate(m_forward,  * justRot;
    return out;
}

glm::vec3 TransformComponent::GetLookRight() const {
    //glm::quat justRot = glm::normalize(glm::quat(m_scene_node->Get().ToParent()));
    glm::quat justRot = glm::normalize(glm::quat(m_scene_node->Get().FromParent()));
    glm::vec3 out = m_right * justRot;
    return out;
}

glm::vec3 TransformComponent::GetLookUp() const {
    //glm::quat justRot = glm::normalize(glm::quat(m_scene_node->Get().ToParent()));
    glm::quat justRot = glm::normalize(glm::quat(m_scene_node->Get().FromParent()));
    glm::vec3 out = m_up * justRot;
    return out;
}

glm::vec3 TransformComponent::GetForward3f() const {
    return glm::vec3(m_forward.x, m_forward.y, m_forward.z);
}

const glm::vec4& TransformComponent::GetForward4f() const {
    return m_forward;
}

glm::vec3 TransformComponent::GetUp3f() const {
    return glm::vec3(m_up.x, m_up.y, m_up.z);
}

const glm::vec4& TransformComponent::GetUp4f() const {
    return m_up;
}

glm::vec3 TransformComponent::GetRight3f() const {
    return glm::vec3(m_right.x, m_right.y, m_right.z);
}

const glm::vec4& TransformComponent::GetRight4f() const {
    return m_right;
}

bool TransformComponent::Init(const pugi::xml_node& data) {
    glm::vec position = posfromattr3f(data.child("Position"));
    glm::vec yawPitchRoll = anglesfromattr3f(data.child("YawPitchRoll"));
    glm::vec scale = posfromattr3f(data.child("Scale"), {1.0f, 1.0f, 1.0f});

    glm::mat4x4 translation_xm = glm::translate(glm::mat4(1.0f), glm::vec3(position.x, position.y, position.z));
    glm::mat4x4 rotation_xm = glm::eulerAngleYXZ(yawPitchRoll.x, yawPitchRoll.y, yawPitchRoll.z);
    glm::mat4x4 scale_xm = glm::scale(glm::mat4(1.0f), glm::vec3(scale.y, scale.x, scale.z));

    glm::mat4x4 result = translation_xm * rotation_xm * scale_xm;

    m_scene_node->SetTransform(result);

	m_initialized = true;

    return m_initialized;
}

glm::vec3 TransformComponent::GetYawPitchRoll() const {
    glm::vec3 res;
    glm::extractEulerAngleXYZ(m_scene_node->Get().ToParent(), res.x, res.y, res.z);
    return res;
}

bool TransformComponent::VInit(const pugi::xml_node& pData) {
    return Init(pData);
}

std::shared_ptr<SceneNode> TransformComponent::GetSceneNode() {
    return m_scene_node;
}

Scene::NodeIndex TransformComponent::GetSceneNodeIndex() {
    return m_scene_node->VGetNodeIndex();
}

glm::vec3 TransformComponent::GetDefaultForward3f() {
    return glm::vec3(DEFAULT_FORWARD_VECTOR.x, DEFAULT_FORWARD_VECTOR.y, DEFAULT_FORWARD_VECTOR.z);
}

glm::vec3 TransformComponent::GetDefaultUp3f() {
    return glm::vec3(DEFAULT_UP_VECTOR.x, DEFAULT_UP_VECTOR.y, DEFAULT_UP_VECTOR.z);
}

glm::vec3 TransformComponent::GetDefaultRight3f() {
    return glm::vec3(DEFAULT_RIGHT_VECTOR.x, DEFAULT_RIGHT_VECTOR.y, DEFAULT_RIGHT_VECTOR.z);
}