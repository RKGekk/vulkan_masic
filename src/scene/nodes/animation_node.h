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
#include <unordered_map>

#include "scene_node.h"
#include "../../animation/matrix_animation.h"

class AnimationNode : public SceneNode {
public:
    using AnimationName = std::string;

	AnimationNode(std::shared_ptr<Scene> scene, Scene::NodeIndex node_index);
	AnimationNode(std::shared_ptr<Scene> scene, std::string name, Scene::NodeIndex parent = 0u);
	AnimationNode(std::shared_ptr<Scene> scene, std::string name, glm::mat4x4 transform, Scene::NodeIndex parent = 0u);

	virtual bool VOnRestore() override;
	virtual bool VOnUpdate() override;

    float GetTotalAnimationTime(const AnimationName& name) const;
    GameTimerDelta GetTotalAnimationDuration(const AnimationName& name) const;

    void addAnimation(const AnimationName& animation_name, std::shared_ptr<MatrixAnimation> animation);
    const std::shared_ptr<MatrixAnimation>& getAnimation(const AnimationName& animation_name) const;
    const std::unordered_map<AnimationName, std::shared_ptr<MatrixAnimation>>& getAnimationMap() const;

private:
    std::unordered_map<AnimationName, std::shared_ptr<MatrixAnimation>> m_animation_map;
};