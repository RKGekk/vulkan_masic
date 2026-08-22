#include "animation_manager.h"

#include <algorithm>
#include <cmath>

const AnimationManager::SequenceName AnimationManager::DEFAULT_SEQUENCE_NAME = "default_sequence";
const std::shared_ptr<AnimationManager::TrackData> NO_TRACK = nullptr;

AnimationManager::AnimationManager(){
    m_sequences[DEFAULT_SEQUENCE_NAME] = std::make_shared<AnimationSequence>();

    m_seq_name_to_state_map[DEFAULT_SEQUENCE_NAME] = SequenceState::Stoped;
    m_seq_state_to_name_map[SequenceState::Stoped].insert(DEFAULT_SEQUENCE_NAME);

    const std::shared_ptr<AnimationSequence>& default_seq = m_sequences[DEFAULT_SEQUENCE_NAME];
    default_seq->sequence_name = DEFAULT_SEQUENCE_NAME;
    default_seq->state = SequenceState::Stoped;
    default_seq->sequence_current_time = 0.0f;
    default_seq->sequence_total_time = 0.0f;
    default_seq->delta_time = 0.0f;
}

void AnimationManager::Update(const GameTimerDelta& delta) {
    if(!m_seq_state_to_name_map.contains(SequenceState::Playing) || m_seq_state_to_name_map[SequenceState::Playing].empty()) return;

    for(const SequenceName& seq_name : m_seq_state_to_name_map[SequenceState::Playing]) {
        if(!m_sequences.contains(seq_name)) continue;

        const std::shared_ptr<AnimationSequence>& seq_ptr = m_sequences[seq_name];
        seq_ptr->delta_time = delta.fGetDeltaSeconds();
        seq_ptr->sequence_current_time += seq_ptr->delta_time;
        seq_ptr->sequence_current_time = std::fmodf(seq_ptr->sequence_current_time, seq_ptr->sequence_total_time);
        
        ProcessSequence(seq_ptr);
    }
}

void AnimationManager::AddNodeAnimation(std::shared_ptr<AnimationNode> animation_node) {
    for(const auto&[anim_name, matrix_anim] : animation_node->getAnimationMap()) {
        m_anim_name_to_node_map[anim_name].insert(animation_node);

        float total_time = matrix_anim->GetTotalAnimationTime();
        if(m_clip_name_to_total_time_map.contains(anim_name)) {
            float old_total_time = m_clip_name_to_total_time_map[anim_name];
            m_clip_name_to_total_time_map[anim_name] = old_total_time > total_time ? old_total_time : total_time;
        }
        else {
            m_clip_name_to_total_time_map[anim_name] = total_time;
        }
    }
}

void AnimationManager::AddClipToDefaultSequence(const ClipName& clip_name, float blend_factor, float animation_speed, float clip_current_time) {
    if(!m_anim_name_to_node_map.contains(clip_name)) return;

    const std::shared_ptr<AnimationSequence>& seq = m_sequences[DEFAULT_SEQUENCE_NAME];
    std::shared_ptr<TrackData> track_data = std::make_shared<TrackData>();
    track_data->clip_name = clip_name;
    track_data->clip_total_time = CountClipTotalTime(clip_name);
    track_data->clip_current_time = std::fmodf(clip_current_time, track_data->clip_total_time);
    track_data->animation_speed = animation_speed;
    seq->sequence_total_time = track_data->clip_total_time > seq->sequence_total_time ? track_data->clip_total_time : seq->sequence_total_time;
    for(const std::shared_ptr<AnimationNode>& anim_node : m_anim_name_to_node_map[clip_name]) {
        track_data->animation_blend_factors[anim_node] = blend_factor;
    }

    seq->data_tracks[clip_name] = std::move(track_data);
}

void AnimationManager::RemoveClipFromDefaultSequence(const ClipName& clip_name) {
    if(!m_anim_name_to_node_map.contains(clip_name)) return;

    const std::shared_ptr<AnimationSequence>& seq = m_sequences[DEFAULT_SEQUENCE_NAME];
    seq->data_tracks.erase(clip_name);
}

void AnimationManager::AddClipToSequence(const SequenceName& seq_name, const ClipName& clip_name) {
    if(!m_anim_name_to_node_map.contains(clip_name) || !m_sequences.contains(seq_name)) return;

    const std::shared_ptr<AnimationSequence>& seq = m_sequences[seq_name];
    std::shared_ptr<TrackData> track_data = std::make_shared<TrackData>();
    track_data->clip_name = clip_name;
    track_data->clip_current_time = 0.0f;
    track_data->clip_total_time = CountClipTotalTime(clip_name);
    track_data->animation_speed = 1.0f;
    seq->sequence_total_time = track_data->clip_total_time > seq->sequence_total_time ? track_data->clip_total_time : seq->sequence_total_time;
    for(const std::shared_ptr<AnimationNode>& anim_node : m_anim_name_to_node_map[clip_name]) {
        track_data->animation_blend_factors[anim_node] = 1.0f;
    }

    seq->data_tracks[clip_name] = std::move(track_data);
}

void AnimationManager::RemoveClipFromSequence(const SequenceName& seq_name, const ClipName& clip_name) {
    if(!m_anim_name_to_node_map.contains(clip_name) || !m_sequences.contains(seq_name)) return;

    const std::shared_ptr<AnimationSequence>& seq = m_sequences[seq_name];
    seq->data_tracks.erase(clip_name);
}

void AnimationManager::Pause(const SequenceName& seq_name) {
    if(m_seq_name_to_state_map.at(seq_name) == SequenceState::Paused) return;

    SequenceState current_state = m_seq_name_to_state_map[seq_name];
    m_seq_state_to_name_map[current_state].erase(seq_name);
    m_seq_state_to_name_map[SequenceState::Paused].insert(seq_name);
    m_seq_name_to_state_map[seq_name] = SequenceState::Paused;
}

void AnimationManager::Stop(const SequenceName& seq_name) {
    if(m_seq_name_to_state_map.at(seq_name) == SequenceState::Stoped) return;

    SequenceState current_state = m_seq_name_to_state_map[seq_name];
    m_seq_state_to_name_map[current_state].erase(seq_name);
    m_seq_state_to_name_map[SequenceState::Stoped].insert(seq_name);
    m_seq_name_to_state_map[seq_name] = SequenceState::Stoped;

    const std::shared_ptr<AnimationSequence>& seq = m_sequences[seq_name];
    seq->sequence_current_time = 0.0f;
    seq->delta_time = 0.0f;
    ProcessSequence(seq);
}

void AnimationManager::Play(const SequenceName& seq_name) {
    if(m_seq_name_to_state_map.at(seq_name) == SequenceState::Playing) return;

    SequenceState current_state = m_seq_name_to_state_map[seq_name];
    m_seq_state_to_name_map[current_state].erase(seq_name);
    m_seq_state_to_name_map[SequenceState::Playing].insert(seq_name);
    m_seq_name_to_state_map[seq_name] = SequenceState::Playing;
}

void AnimationManager::SetClipAnimationSpeed(const SequenceName& seq_name, const ClipName& clip_name, float p) {
    if(!m_anim_name_to_node_map.contains(clip_name) || !m_sequences.contains(seq_name)) return;

    const std::shared_ptr<AnimationSequence>& seq = m_sequences[seq_name];
    if(!seq->data_tracks.contains(clip_name)) return;

    seq->data_tracks[clip_name]->animation_speed = p;
}

void AnimationManager::SetClipTotalTime(const SequenceName& seq_name, const ClipName& clip_name, float t) {
    if(!m_anim_name_to_node_map.contains(clip_name) || !m_sequences.contains(seq_name)) return;

    const std::shared_ptr<AnimationSequence>& seq = m_sequences[seq_name];
    if(!seq->data_tracks.contains(clip_name)) return;

    seq->data_tracks[clip_name]->clip_total_time = t;
}

void AnimationManager::SetClipCurrentTime(const SequenceName& seq_name, const ClipName& clip_name, float t) {
    if(!m_anim_name_to_node_map.contains(clip_name) || !m_sequences.contains(seq_name)) return;

    const std::shared_ptr<AnimationSequence>& seq = m_sequences[seq_name];
    if(!seq->data_tracks.contains(clip_name)) return;

    const std::shared_ptr<TrackData>& trk = seq->data_tracks[clip_name];
    //trk->clip_current_time = std::fmodf(t, trk->clip_total_time * trk->animation_speed) * trk->animation_speed;
    trk->clip_current_time = std::fmodf(t, trk->clip_total_time) * trk->animation_speed;

    if(seq->state == SequenceState::Paused) {
        seq->delta_time = 0.0f;
        ProcessSequence(seq);
    }
}

void AnimationManager::SetClipBlendFactor(const SequenceName& seq_name, const ClipName& clip_name, BlendFactor k) {
    if(!m_anim_name_to_node_map.contains(clip_name) || !m_sequences.contains(seq_name)) return;

    const std::shared_ptr<AnimationSequence>& seq = m_sequences[seq_name];
    if(!seq->data_tracks.contains(clip_name)) return;

    const std::shared_ptr<TrackData>& trk = seq->data_tracks[clip_name];
    for(auto&[anim_node, blend_factor] : trk->animation_blend_factors) {
        blend_factor = k;
    }

    if(seq->state == SequenceState::Paused) {
        seq->delta_time = 0.0f;
        ProcessSequence(seq);
    }
}

void AnimationManager::SetSequenceCurrentTime(const SequenceName& seq_name, float t) {
    if(!m_sequences.contains(seq_name)) return;

    const std::shared_ptr<AnimationSequence>& seq = m_sequences[seq_name];
    seq->sequence_current_time = std::fmodf(t, seq->sequence_total_time);

    if(seq->state == SequenceState::Paused) {
        seq->delta_time = 0.0f;
        ProcessSequence(seq);
    }
}

void AnimationManager::SetSequenceTotalTime(const SequenceName& seq_name, float t) {
    if(!m_sequences.contains(seq_name)) return;

    const std::shared_ptr<AnimationSequence>& seq = m_sequences[seq_name];
    seq->sequence_total_time = t;
}

float AnimationManager::GetClipAnimationSpeed(const SequenceName& seq_name, const ClipName& clip_name) const {
    if(!m_anim_name_to_node_map.contains(clip_name) || !m_sequences.contains(seq_name)) return 1.0f;

    const std::shared_ptr<AnimationSequence>& seq = m_sequences.at(seq_name);
    if(!seq->data_tracks.contains(clip_name)) return 1.0f;

    return seq->data_tracks[clip_name]->animation_speed;
}

float AnimationManager::GetClipCurrentTime(const SequenceName& seq_name, const ClipName& clip_name) const {
    if(!m_anim_name_to_node_map.contains(clip_name) || !m_sequences.contains(seq_name)) return 0.0f;

    const std::shared_ptr<AnimationSequence>& seq = m_sequences.at(seq_name);
    if(!seq->data_tracks.contains(clip_name)) return 0.0f;

    return seq->data_tracks[clip_name]->clip_current_time;
}

float AnimationManager::GetClipCurrentNormPos(const SequenceName& seq_name, const ClipName& clip_name) const {
    if(!m_anim_name_to_node_map.contains(clip_name) || !m_sequences.contains(seq_name)) return 0.0f;

    const std::shared_ptr<AnimationSequence>& seq = m_sequences.at(seq_name);
    if(!seq->data_tracks.contains(clip_name)) return 0.0f;
    
    const std::shared_ptr<TrackData>& trk = seq->data_tracks[clip_name];
    // float total_anim_time = GetClipTotalTime(clip_name);
    float total_anim_time = trk->clip_total_time;
    if(total_anim_time == 0.0f) return 0.0f;

	return trk->clip_current_time / total_anim_time;
}

float AnimationManager::GetSequenceCurrentTime(const SequenceName& seq_name, const ClipName& clip_name) const {
    if(!m_sequences.contains(seq_name)) return 0.0f;
    return m_sequences.at(seq_name)->sequence_current_time;
}

float AnimationManager::GetSequenceCurrentNormPos(const SequenceName& seq_name, const ClipName& clip_name) const {
    if(!m_sequences.contains(seq_name)) return 0.0f;
    const std::shared_ptr<AnimationSequence>& seq = m_sequences.at(seq_name);
    if(seq->sequence_total_time == 0.0f) return 0.0f;
    return seq->sequence_current_time / seq->sequence_total_time;
}


float AnimationManager::GetClipTotalTime(const ClipName& clip_name) const {
    return m_clip_name_to_total_time_map.at(clip_name);
}

GameTimerDelta AnimationManager::GetClipTotalDuration(const ClipName& clip_name) const {
    GameTimerDelta dt;
    dt.ResetDuration();
    dt.AddDeltaDuration(m_clip_name_to_total_time_map.at(clip_name));
    return dt;
}

const std::unordered_map<AnimationManager::ClipName, std::unordered_set<std::shared_ptr<AnimationNode>>>& AnimationManager::GetClipMap() const {
    return m_anim_name_to_node_map;
}

const std::shared_ptr<AnimationManager::AnimationSequence>& AnimationManager::GetAnimationSequence(const SequenceName& seq_name) const {
    return m_sequences.at(seq_name);
}

const std::unordered_map<AnimationManager::SequenceName, std::shared_ptr<AnimationManager::AnimationSequence>>& AnimationManager::GetAnimationSequenceMap() const {
    return m_sequences;
}

const std::shared_ptr<AnimationManager::TrackData>& AnimationManager::getTrack(const SequenceName& seq_name, const ClipName& clip_name) const {
    const std::shared_ptr<AnimationSequence>& seq = m_sequences.at(seq_name);
    if(!seq->data_tracks.contains(clip_name)) return NO_TRACK;
    return seq->data_tracks[clip_name];
}

void AnimationManager::ProcessSequence(const std::shared_ptr<AnimationSequence>& seq) {
    for(const auto&[clip_name, track_data] : seq->data_tracks) {
        //track_data->clip_total_time = track_data->clip_total_time * track_data->animation_speed;
        track_data->clip_current_time += seq->delta_time * track_data->animation_speed;
        track_data->clip_current_time = std::fmodf(track_data->clip_current_time, track_data->clip_total_time);
        float clip_time = track_data->clip_current_time;
        for(const auto[anim_node, blend_factor] : track_data->animation_blend_factors) {
            glm::mat4x4 transform = anim_node->Get().ToParent();
            anim_node->getAnimation(track_data->clip_name)->InterpolateTime(clip_time, transform, blend_factor);
	        anim_node->SetTransform(transform); 
        }
    }
}

float AnimationManager::CountClipTotalTime(const ClipName& clip_name) const {
    if(!m_anim_name_to_node_map.contains(clip_name)) return 0.0f;
    float max_time = 0.0f;
    for(const std::shared_ptr<AnimationNode>& anim_node : m_anim_name_to_node_map.at(clip_name)) {
        float total_time = anim_node->GetTotalAnimationTime(clip_name);
        if(total_time > max_time) max_time = total_time;
    }
    return max_time;
}

void AnimationManager::CalcAnimRoots(ClipName clip_name) {
    if(!m_anim_name_to_node_map.contains(clip_name)) return;

    const std::unordered_set<std::shared_ptr<AnimationNode>>& all_anim_nodes = m_anim_name_to_node_map[clip_name];
    if(!all_anim_nodes.size()) return;

    const std::shared_ptr<Scene>& scene = (*(all_anim_nodes.begin()))->GetScene();
    std::vector<std::shared_ptr<AnimationNode>>& anim_roots = m_anim_roots[clip_name];
    anim_roots.clear();
    for(const std::shared_ptr<AnimationNode> anim_node : all_anim_nodes) {
        const std::shared_ptr<SceneNode>& parent_node = anim_node->GetParent();
        if(std::shared_ptr<AnimationNode> parent_anim = std::dynamic_pointer_cast<AnimationNode>(scene->getProperty(parent_node->VGetNodeIndex(), Scene::NODE_TYPE_FLAG_ANIMATION))) {
            if(!all_anim_nodes.contains(parent_anim)) {
                anim_roots.push_back(anim_node);
            }
        }
        else {
            anim_roots.push_back(anim_node);
        }
    }
}

const std::vector<std::shared_ptr<AnimationNode>>& AnimationManager::GetClipRoots(const ClipName& name) const {
    return m_anim_roots.at(name);
}