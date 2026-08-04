#include "animation_node.h"

AnimationNode::AnimationNode(std::shared_ptr<Scene> scene, Scene::NodeIndex node_index) : SceneNode(std::move(scene), node_index) {};
AnimationNode::AnimationNode(std::shared_ptr<Scene> scene, std::string name, Scene::NodeIndex parent) : SceneNode(std::move(scene), std::move(name), parent) {};
AnimationNode::AnimationNode(std::shared_ptr<Scene> scene, std::string name, glm::mat4x4 transform, Scene::NodeIndex parent) : SceneNode(std::move(scene), std::move(name), transform, parent) {};

bool AnimationNode::VOnRestore() {
    return SceneNode::VOnRestore();
};

bool AnimationNode::VOnUpdate() {
    return SceneNode::VOnUpdate();
};

float AnimationNode::GetTotalAnimationTime(const AnimationName& name) const {
    const std::shared_ptr<MatrixAnimation>& anim = m_animation_map.at(name);
	float t1 = anim->RotationKeyframes.size() > 0u ? anim->RotationKeyframes.back().TimePos : 0.0f;
	float t2 = anim->TranslationKeyframes.size() > 0u ? anim->TranslationKeyframes.back().TimePos : 0.0f;
    return t1 > t2 ? t1 : t2;
}

GameTimerDelta AnimationNode::GetTotalAnimationDuration(const AnimationName& name) const {
    const std::shared_ptr<MatrixAnimation>& anim = m_animation_map.at(name);
    float t1 = anim->RotationKeyframes.size() > 0u ? anim->RotationKeyframes.back().TimePos : 0.0f;
	float t2 = anim->TranslationKeyframes.size() > 0u ? anim->TranslationKeyframes.back().TimePos : 0.0f;
	float t = t1 > t2 ? t1 : t2;
	GameTimerDelta dt;
	dt.AddDeltaDuration(t);
    return dt;
}

void AnimationNode::addAnimation(const AnimationName& animation_name, std::shared_ptr<MatrixAnimation> animation) {
    m_animation_map[animation_name] = std::move(animation);
}

const std::shared_ptr<MatrixAnimation>& AnimationNode::getAnimation(const AnimationName& animation_name) const {
    return m_animation_map.at(animation_name);
}

const std::unordered_map<AnimationNode::AnimationName, std::shared_ptr<MatrixAnimation>>& AnimationNode::getAnimationMap() const {
    return m_animation_map;
}