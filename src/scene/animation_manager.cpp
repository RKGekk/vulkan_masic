#include "animation_manager.h"

AnimationManager::AnimationManager(){}

void AnimationManager::Update(const GameTimerDelta& delta) {
    if(!m_state_to_name_map.contains(ClipState::Playing) || m_state_to_name_map[ClipState::Playing].empty()) return;

    for(const ClipName& clip_name : m_state_to_name_map[ClipState::Playing]) {
        m_name_to_current_time_map[clip_name].AddDeltaDuration(delta);
        float current_time = m_name_to_current_time_map[clip_name].fGetTotalSeconds();
        ProcessClip(clip_name, current_time);
    }
}

void AnimationManager::AddNodeAnimation(std::shared_ptr<AnimationNode> animation_node) {
    m_animated_nodes.insert(std::move(animation_node));
}

void AnimationManager::AddNodeAnimationClip(std::shared_ptr<AnimationClip> clip) {
    m_animations[clip->clip_name] = clip;
    float total_time = CountClipTotalTime(clip->clip_name);
    GameTimerDelta dt;
	dt.AddDeltaDuration(total_time);
    m_name_to_total_time_map[clip->clip_name] = dt;
}

void AnimationManager::Pause(const ClipName& clip_name) {
    if(!m_name_to_state_map.contains(clip_name) || m_name_to_state_map.at(clip_name) == ClipState::Paused) return;

    ClipState current_state = m_name_to_state_map[clip_name];
    m_state_to_name_map[current_state].erase(clip_name);
    m_state_to_name_map[ClipState::Paused].insert(clip_name);
    m_name_to_state_map[clip_name] = ClipState::Paused;
}

void AnimationManager::Stop(const ClipName& clip_name) {
    if(!m_name_to_state_map.contains(clip_name) || m_name_to_state_map.at(clip_name) == ClipState::Stoped) return;

    ClipState current_state = m_name_to_state_map[clip_name];
    m_state_to_name_map[current_state].erase(clip_name);
    m_state_to_name_map[ClipState::Stoped].insert(clip_name);
    m_name_to_state_map[clip_name] = ClipState::Stoped;
    m_name_to_current_time_map[clip_name].ResetDuration();

    ProcessClip(clip_name, 0.0f);
}

void AnimationManager::Play(const ClipName& clip_name) {
    if(!m_name_to_state_map.contains(clip_name) || m_name_to_state_map.at(clip_name) == ClipState::Playing) return;

    ClipState current_state = m_name_to_state_map[clip_name];
    m_state_to_name_map[current_state].erase(clip_name);
    m_state_to_name_map[ClipState::Playing].insert(clip_name);
    m_name_to_state_map[clip_name] = ClipState::Playing;
}

void AnimationManager::SetClipCurrentTime(const ClipName& clip_name, float t) {
    if(!m_name_to_state_map.contains(clip_name)) return;

    GameTimerDelta dt;
	dt.AddDeltaDuration(t);
    m_name_to_current_time_map[clip_name].ResetDuration();
    m_name_to_current_time_map[clip_name].AddDeltaDuration(dt);

    if (m_name_to_state_map[clip_name] != ClipState::Stoped) {
        ProcessClip(clip_name, t);
    }
}

void AnimationManager::SetClipCurrentDuration(const ClipName& clip_name, const GameTimerDelta& duration) {
    if(!m_name_to_state_map.contains(clip_name)) return;

    float t = m_name_to_current_time_map[clip_name].fGetTotalSeconds();
    SetClipCurrentTime(clip_name, t);
}

float AnimationManager::GetClipCurrentTime(const ClipName& clip_name) const {
    if(!m_name_to_state_map.contains(clip_name)) return 0.0f;
    return m_name_to_current_time_map.at(clip_name).fGetTotalSeconds();
}

float AnimationManager::GetClipCurrentNormPos(const ClipName& clip_name) const {
    if(!m_name_to_state_map.contains(clip_name)) return 0.0f;
    float total_anim_time = GetClipTotalTime(clip_name);
    if(total_anim_time == 0.0f) return 0.0f;
	return m_name_to_current_time_map.at(clip_name).GetTotalSeconds() / total_anim_time;
}

const GameTimerDelta& AnimationManager::GetClipCurrentDuration(const ClipName& clip_name) const {
    return m_name_to_current_time_map.at(clip_name);
}

float AnimationManager::GetClipTotalTime(const ClipName& clip_name) const {
    return m_name_to_total_time_map.at(clip_name).fGetTotalSeconds();
}

GameTimerDelta AnimationManager::GetClipTotalDuration(const ClipName& clip_name) const {
    return m_name_to_total_time_map.at(clip_name);
}

const std::unordered_map<AnimationManager::ClipName, std::shared_ptr<AnimationManager::AnimationClip>>& AnimationManager::GetClipMap() const {
    return m_animations;
}

void AnimationManager::ProcessClip(const ClipName& clip_name, float t) {
    for(const auto&[anim_name, anim_node] : m_animations[clip_name]->animations) {
        glm::mat4x4 transform = glm::mat4x4(1.0f);
        anim_node->getAnimation(anim_name)->InterpolateTime(t, transform);
	    anim_node->SetTransform(transform);
    }
}

float AnimationManager::CountClipTotalTime(const ClipName& clip_name) const {
    if(!m_name_to_state_map.contains(clip_name)) return 0.0f;
    float max_time = 0.0f;
    for(const auto&[anim_name, anim_node] : m_animations.at(clip_name)->animations) {
        float total_time = anim_node->GetTotalAnimationTime(anim_name);
        if(total_time > max_time) max_time = total_time;
    }
    return max_time;
}
