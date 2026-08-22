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
    using SequenceName = std::string;
    using BlendFactor = float;

    static const SequenceName DEFAULT_SEQUENCE_NAME;

    enum class SequenceState {
		Playing,
		Stoped,
        Paused
	};

    struct TrackData {
        ClipName clip_name;
        float clip_current_time;
        float clip_total_time;
        float animation_speed;
        std::unordered_map<std::shared_ptr<AnimationNode>, BlendFactor> animation_blend_factors;
    };

    struct AnimationSequence {
        SequenceName sequence_name;
        SequenceState state;
        float sequence_current_time;
        float sequence_total_time;
        std::unordered_map<ClipName, std::shared_ptr<TrackData>> data_tracks;
    };

    AnimationManager();
    void Update(const GameTimerDelta& delta);
    void CalcAnimRoots(ClipName clip_name);

    void AddNodeAnimation(std::shared_ptr<AnimationNode> animation_node);

    void AddClipToDefaultSequence(const ClipName& name, float blend_factor = 1.0f, float animation_speed = 1.0f, float clip_current_time = 0.0f);
    void RemoveClipFromDefaultSequence(const ClipName& name);
    void AddClipToSequence(const SequenceName& seq_name, const ClipName& name);
    void RemoveClipFromSequence(const SequenceName& seq_name, const ClipName& name);

    void Pause(const SequenceName& seq_name);
    void Stop(const SequenceName& seq_name);
    void Play(const SequenceName& seq_name);

    void SetClipAnimationSpeed(const SequenceName& seq_name, const ClipName& clip_name, float p);
    void SetClipTotalTime(const SequenceName& seq_name, const ClipName& clip_name, float t);
    void SetClipCurrentTime(const SequenceName& seq_name, const ClipName& clip_name, float t);
    void SetClipBlendFactor(const SequenceName& seq_name, const ClipName& clip_name, BlendFactor k);
    void SetSequenceCurrentTime(const SequenceName& seq_name, float t);
    void SetSequenceTotalTime(const SequenceName& seq_name, float t);

    float GetClipAnimationSpeed(const SequenceName& seq_name, const ClipName& clip_name) const;
    float GetClipCurrentTime(const SequenceName& seq_name, const ClipName& clip_name) const;
    float GetClipCurrentNormPos(const SequenceName& seq_name, const ClipName& clip_name) const;
    
    float GetSequenceCurrentTime(const SequenceName& seq_name, const ClipName& clip_name) const;
    float GetSequenceCurrentNormPos(const SequenceName& seq_name, const ClipName& clip_name) const;


    float GetClipTotalTime(const ClipName& name) const;
    GameTimerDelta GetClipTotalDuration(const ClipName& name) const;

    const std::unordered_map<ClipName, std::unordered_set<std::shared_ptr<AnimationNode>>>& GetClipMap() const;
    const std::vector<std::shared_ptr<AnimationNode>>& GetClipRoots(const ClipName& name) const;
    const std::shared_ptr<AnimationSequence>& GetAnimationSequence(const SequenceName& seq_name) const;
    const std::unordered_map<SequenceName, std::shared_ptr<AnimationSequence>>& GetAnimationSequenceMap() const;
    const std::shared_ptr<TrackData>& getTrack(const SequenceName& seq_name, const ClipName& clip_name) const;

private:
    void ProcessSequence(const std::shared_ptr<AnimationSequence>& seq);
    float CountClipTotalTime(const ClipName& name) const;

    std::unordered_map<SequenceState, std::unordered_set<SequenceName>> m_seq_state_to_name_map;
    std::unordered_map<SequenceName, SequenceState> m_seq_name_to_state_map;

    std::unordered_map<SequenceName, std::shared_ptr<AnimationSequence>> m_sequences;

    std::unordered_map<ClipName, float> m_clip_name_to_total_time_map;
    std::unordered_map<ClipName, std::unordered_set<std::shared_ptr<AnimationNode>>> m_anim_name_to_node_map;
    std::unordered_map<ClipName, std::vector<std::shared_ptr<AnimationNode>>> m_anim_roots;
};