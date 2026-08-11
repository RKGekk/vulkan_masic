#include "animation_manager.h"

AnimationManager::AnimationManager(){}

void AnimationManager::Update(const GameTimerDelta& delta) {
    if(!m_state_to_name_map.contains(ClipState::Playing) || m_state_to_name_map[ClipState::Playing].empty()) return;

    for(const ClipName& clip_name : m_state_to_name_map[ClipState::Playing]) {
        m_name_to_current_time_map[clip_name] += delta.fGetDeltaSeconds();
        float current_time = m_name_to_current_time_map[clip_name];
        ProcessClip(clip_name, current_time);
    }
}

void AnimationManager::AddNodeAnimation(std::shared_ptr<AnimationNode> animation_node) {
    for(const auto&[anim_name, matrix_anim] : animation_node->getAnimationMap()) {
        m_anim_name_to_node_map[anim_name].insert(animation_node);
        m_name_to_state_map[anim_name] = ClipState::Stoped;
        m_state_to_name_map[ClipState::Stoped].insert(anim_name);
        m_name_to_current_time_map[anim_name] = 0.0f;
        float total_time = matrix_anim->GetTotalAnimationTime();
        if(m_name_to_total_time_map.contains(anim_name)) {
            float old_total_time = m_name_to_total_time_map[anim_name];
            m_name_to_total_time_map[anim_name] = old_total_time > total_time ? old_total_time : total_time;
        }
        else {
            m_name_to_total_time_map[anim_name] = total_time;
        }
        
    }
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
    m_name_to_current_time_map[clip_name] = 0.0f;

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

    m_name_to_current_time_map[clip_name] = t;

    if (m_name_to_state_map[clip_name] != ClipState::Stoped) {
        ProcessClip(clip_name, t);
    }
}

void AnimationManager::SetClipCurrentDuration(const ClipName& clip_name, const GameTimerDelta& duration) {
    if(!m_name_to_state_map.contains(clip_name)) return;

    float t = m_name_to_current_time_map[clip_name];
    SetClipCurrentTime(clip_name, t);
}

float AnimationManager::GetClipCurrentTime(const ClipName& clip_name) const {
    if(!m_name_to_state_map.contains(clip_name)) return 0.0f;
    return m_name_to_current_time_map.at(clip_name);
}

float AnimationManager::GetClipCurrentNormPos(const ClipName& clip_name) const {
    if(!m_name_to_state_map.contains(clip_name)) return 0.0f;
    float total_anim_time = GetClipTotalTime(clip_name);
    if(total_anim_time == 0.0f) return 0.0f;
	return m_name_to_current_time_map.at(clip_name) / total_anim_time;
}

GameTimerDelta AnimationManager::GetClipCurrentDuration(const ClipName& clip_name) const {
    GameTimerDelta dt;
    dt.ResetDuration();
    dt.AddDeltaDuration(m_name_to_current_time_map.at(clip_name));
    return dt;
}

float AnimationManager::GetClipTotalTime(const ClipName& clip_name) const {
    return m_name_to_total_time_map.at(clip_name);
}

GameTimerDelta AnimationManager::GetClipTotalDuration(const ClipName& clip_name) const {
    GameTimerDelta dt;
    dt.ResetDuration();
    dt.AddDeltaDuration(m_name_to_total_time_map.at(clip_name));
    return dt;
}

const std::unordered_map<AnimationManager::ClipName, std::unordered_set<std::shared_ptr<AnimationNode>>>& AnimationManager::GetClipMap() const {
    return m_anim_name_to_node_map;
}

void AnimationManager::ProcessClip(const ClipName& clip_name, float t) {
    for(const std::shared_ptr<AnimationNode>& anim_node : m_anim_name_to_node_map[clip_name]) {
        glm::mat4x4 transform = glm::mat4x4(1.0f);
        anim_node->getAnimation(clip_name)->InterpolateTime(t, transform);
	    anim_node->SetTransform(transform);
    }
}

float AnimationManager::CountClipTotalTime(const ClipName& clip_name) const {
    if(!m_name_to_state_map.contains(clip_name)) return 0.0f;
    float max_time = 0.0f;
    for(const std::shared_ptr<AnimationNode>& anim_node : m_anim_name_to_node_map.at(clip_name)) {
        float total_time = anim_node->GetTotalAnimationTime(clip_name);
        if(total_time > max_time) max_time = total_time;
    }
    return max_time;
}
