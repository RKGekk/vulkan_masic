#include "light_node.h"

LightNode::LightNode(std::shared_ptr<Scene> scene, Scene::NodeIndex node_index) : SceneNode(std::move(scene), node_index) {
    SetNodeType(Scene::NODE_TYPE_FLAG_LIGHT);
}

LightNode::LightNode(std::shared_ptr<Scene> scene, std::string name, Scene::NodeIndex parent) 
    : SceneNode(std::move(scene), std::move(name), parent)
    , m_light_type(LightType::SPOT)
    , m_strength(1.0f)
    , m_falloff_start(0.0f)
    , m_falloff_end(1.0f)
    , m_spot_power(1.0f)
    , m_outer_angle(glm::pi<float>())
    , m_inner_angle(glm::pi<float>()) {
    SetNodeType(Scene::NODE_TYPE_FLAG_LIGHT);
}

LightNode::LightNode(std::shared_ptr<Scene> scene, std::string name, glm::mat4x4 camera_transform, Scene::NodeIndex parent)
    : SceneNode(std::move(scene), std::move(name), camera_transform, parent)
    , m_light_type(LightType::SPOT)
    , m_strength(1.0f)
    , m_falloff_start(0.0f)
    , m_falloff_end(1.0f)
    , m_spot_power(1.0f)
    , m_outer_angle(glm::pi<float>())
    , m_inner_angle(glm::pi<float>()) {
    SetNodeType(Scene::NODE_TYPE_FLAG_LIGHT);
}

LightNode::LightNode(std::shared_ptr<Scene> scene, std::string name, glm::mat4x4 camera_transform, const LightNodeProperties& light_props, LightType light_type, Scene::NodeIndex parent)
    : SceneNode(std::move(scene), std::move(name), camera_transform, parent)
    , m_light_type(light_type)
    , m_strength(light_props.strength)
    , m_falloff_start(light_props.falloff_start)
    , m_falloff_end(light_props.falloff_end)
    , m_spot_power(light_props.spot_power)
    , m_outer_angle(light_props.outer_angle)
    , m_inner_angle(light_props.inner_angle) {
    SetNodeType(Scene::NODE_TYPE_FLAG_LIGHT);
}

bool LightNode::VOnRestore() {
    return SceneNode::VOnRestore();
}

bool LightNode::VOnUpdate() {
    return SceneNode::VOnUpdate();
}

void LightNode::setLightType(LightType light_type) {
    m_light_type = light_type;
}

LightNode::LightType LightNode::getLightType() const {
    return m_light_type;
}

void LightNode::SetLightProperties(const LightNodeProperties& light_props) {
    m_strength = light_props.strength;
    m_falloff_start = light_props.falloff_start;
    m_falloff_end = light_props.falloff_end;
    m_spot_power = light_props.spot_power;
    m_outer_angle = light_props.outer_angle;
    m_inner_angle = light_props.inner_angle;
}

LightNodeProperties LightNode::GetLightProperties() const {
    LightNodeProperties light_props {};
    light_props.strength = m_strength;
    light_props.falloff_start = m_falloff_start;
    light_props.falloff_end = m_falloff_end;
    light_props.spot_power = m_spot_power;
    light_props.outer_angle = m_outer_angle;
    light_props.inner_angle = m_inner_angle;

    light_props.position = Get().ToRootTranslation3();
    light_props.direction = Get().ToRootDirection();

    return light_props;
}

void LightNode::setStrength(glm::vec3 strength) {
    m_strength = strength;
}

glm::vec3 LightNode::getStrength() const {
    return m_strength;
}

void LightNode::setFalloffStart(float falloff_start) {
    m_falloff_start = falloff_start;
}

float LightNode::getFalloffStart() const {
    return m_falloff_start;
}

void LightNode::setFalloffEnd(float falloff_end) {
    m_falloff_end = falloff_end;
}

float LightNode::getFalloffEnd() const {
    return m_falloff_end;
}

void LightNode::setSpotPower(float spot_power) {
    m_spot_power = spot_power;
}

float LightNode::getSpotPower() const {
    return m_spot_power;
}

void LightNode::setOuterAngle(float outer_angle) {
    m_outer_angle = outer_angle;
}

float LightNode::getOuterAngle() const {
    return m_outer_angle;
}

void LightNode::setInnerAngle(float inner_angle) {
    m_inner_angle = inner_angle;
}

float LightNode::getInnerAngle() const {
    return m_inner_angle;
}