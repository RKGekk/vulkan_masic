#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/dual_quaternion.hpp>

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "matrix_animation.h"
#include "../scene/scene.h"

class SkinnedData {
public:
    using SkinName = std::string;
    using ClipName = std::string;

    struct AnimationClip {
        std::string clip_name;
        std::unordered_map<Scene::NodeIndex, std::shared_ptr<MatrixAnimation>> bones_animations;
    };

    SkinnedData(SkinName name);

    void setSkeleton(std::shared_ptr<Scene> skeleton_hierarchy, std::shared_ptr<Scene> bind_offsets);
    void addBoneAnimation(ClipName clip_name, Scene::NodeIndex node_idx, std::shared_ptr<MatrixAnimation> bone_anim);
    void addClip(ClipName clip_name, std::shared_ptr<AnimationClip> clip);
    const std::shared_ptr<Scene>& getSkeleton();

private:
    SkinName m_skeleton_name;
    std::shared_ptr<Scene> m_skeleton;
    std::shared_ptr<Scene> m_bind_offsets;
    std::unordered_map<ClipName, std::shared_ptr<AnimationClip>> m_bones_animations;
};