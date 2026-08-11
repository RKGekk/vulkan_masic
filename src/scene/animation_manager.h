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
#include <unordered_set>
#include <utility>
#include <vector>

#include "nodes/animation_node.h"

class AnimationManager {
public:
    using ClipName = std::string;
    enum class ClipState {
		Playing,
		Stoped,
        Paused
	};

    AnimationManager();
    void Update(const GameTimerDelta& delta);

    void AddNodeAnimation(std::shared_ptr<AnimationNode> animation_node);

    void Pause(const ClipName& name);
    void Stop(const ClipName& name);
    void Play(const ClipName& name);

    void SetClipCurrentTime(const ClipName& name, float t);
    void SetClipCurrentDuration(const ClipName& name, const GameTimerDelta& duration);

    float GetClipCurrentTime(const ClipName& name) const;
    float GetClipCurrentNormPos(const ClipName& name) const;
    GameTimerDelta GetClipCurrentDuration(const ClipName& name) const;
    float GetClipTotalTime(const ClipName& name) const;
    GameTimerDelta GetClipTotalDuration(const ClipName& name) const;

    const std::unordered_map<ClipName, std::unordered_set<std::shared_ptr<AnimationNode>>>& GetClipMap() const;

private:
    void ProcessClip(const ClipName& clip_name, float t);
    float CountClipTotalTime(const ClipName& name) const;

    std::unordered_map<ClipState, std::unordered_set<ClipName>> m_state_to_name_map;
    std::unordered_map<ClipName, ClipState> m_name_to_state_map;
    std::unordered_map<ClipName, float> m_name_to_current_time_map;
    std::unordered_map<ClipName, float> m_name_to_total_time_map;
    std::unordered_map<ClipName, std::unordered_set<std::shared_ptr<AnimationNode>>> m_anim_name_to_node_map;
};