#include "animation_manager.h"

#include <algorithm>
#include <cmath>

const AnimationManager::SequenceName AnimationManager::DEFAULT_SEQUENCE_NAME = "default_sequence";

AnimationManager::AnimationManager(){
    m_sequences[DEFAULT_SEQUENCE_NAME] = std::make_shared<AnimationSequence>();

    m_seq_name_to_state_map[DEFAULT_SEQUENCE_NAME] = SequenceState::Stoped;
    m_seq_state_to_name_map[SequenceState::Stoped].insert(DEFAULT_SEQUENCE_NAME);

    const std::shared_ptr<AnimationSequence>& default_seq = m_sequences[DEFAULT_SEQUENCE_NAME];
    default_seq->sequence_name = DEFAULT_SEQUENCE_NAME;
    default_seq->state = SequenceState::Stoped;
    default_seq->sequence_current_time = 0.0f;
    default_seq->sequence_total_time = 0.0f;
}

void AnimationManager::Update(const GameTimerDelta& delta) {
    if(!m_seq_state_to_name_map.contains(SequenceState::Playing) || m_seq_state_to_name_map[SequenceState::Playing].empty()) return;

    for(const SequenceName& seq_name : m_seq_state_to_name_map[SequenceState::Playing]) {
        if(!m_sequences.contains(seq_name)) continue;

        const std::shared_ptr<AnimationSequence>& seq_ptr = m_sequences[seq_name];
        seq_ptr->sequence_current_time += delta.fGetDeltaSeconds();

        float seq_current_time = seq_ptr->sequence_current_time;
        float seq_total_time = seq_ptr->sequence_total_time;
        if(seq_current_time >= seq_total_time && seq_total_time > 0.0f) {
            seq_current_time = std::fmodf(seq_current_time, seq_total_time);
            seq_ptr->sequence_current_time = seq_current_time;
        }
        for (const std::shared_ptr<TrackData>& track_data : seq_ptr->data_tracks) {
            ProcessClip(seq_name, track_data->clip_name, seq_current_time);
        }
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

void AnimationManager::AddClipToDefaultSequence(const ClipName& clip_name) {
    if(!m_anim_name_to_node_map.contains(clip_name)) return;

    const std::shared_ptr<AnimationSequence>& seq = m_sequences[DEFAULT_SEQUENCE_NAME];
    std::shared_ptr<TrackData> track_data = std::make_shared<TrackData>();
    track_data->clip_name = clip_name;
    track_data->clip_current_time = 0.0f;
    track_data->clip_total_time = CountClipTotalTime(clip_name);
    track_data->animation_speed = 1.0f;
    for(const std::shared_ptr<AnimationNode>& anim_node : m_anim_name_to_node_map[clip_name]) {
        track_data->animation_blend_factors[anim_node] = 1.0f;
    }

    seq->data_tracks.push_back(track_data);
}

void AnimationManager::RemoveClipFromDefaultSequence(const ClipName& clip_name) {
    if(!m_anim_name_to_node_map.contains(clip_name)) return;

    const std::shared_ptr<AnimationSequence>& seq = m_sequences[DEFAULT_SEQUENCE_NAME];
    std::vector<std::shared_ptr<TrackData>>::iterator it = std::find_if(seq->data_tracks.begin(), seq->data_tracks.end(),[&clip_name](const std::shared_ptr<TrackData>& val){return val->clip_name == clip_name;});
    if(it != seq->data_tracks.end()) {
        seq->data_tracks.erase(it);
    }
}

void AnimationManager::AddClipToSequence(const SequenceName& seq_name, const ClipName& clip_name) {
    if(!m_anim_name_to_node_map.contains(clip_name) || !m_sequences.contains(seq_name)) return;

    const std::shared_ptr<AnimationSequence>& seq = m_sequences[seq_name];
    std::shared_ptr<TrackData> track_data = std::make_shared<TrackData>();
    track_data->clip_name = clip_name;
    track_data->clip_current_time = 0.0f;
    track_data->clip_total_time = CountClipTotalTime(clip_name);
    track_data->animation_speed = 1.0f;
    for(const std::shared_ptr<AnimationNode>& anim_node : m_anim_name_to_node_map[clip_name]) {
        track_data->animation_blend_factors[anim_node] = 1.0f;
    }

    seq->data_tracks.push_back(track_data);
}

void AnimationManager::RemoveClipFromSequence(const SequenceName& seq_name, const ClipName& clip_name) {
    if(!m_anim_name_to_node_map.contains(clip_name) || !m_sequences.contains(seq_name)) return;

    const std::shared_ptr<AnimationSequence>& seq = m_sequences[seq_name];
    std::vector<std::shared_ptr<TrackData>>::iterator it = std::find_if(seq->data_tracks.begin(), seq->data_tracks.end(),[&clip_name](const std::shared_ptr<TrackData>& val){return val->clip_name == clip_name;});
    if(it != seq->data_tracks.end()) {
        seq->data_tracks.erase(it);
    }
}

void AnimationManager::Pause() {
    if(m_seq_name_to_state_map.at(DEFAULT_SEQUENCE_NAME) == SequenceState::Paused) return;

    SequenceState current_state = m_seq_name_to_state_map[DEFAULT_SEQUENCE_NAME];
    m_seq_state_to_name_map[current_state].erase(DEFAULT_SEQUENCE_NAME);
    m_seq_state_to_name_map[SequenceState::Paused].insert(DEFAULT_SEQUENCE_NAME);
    m_seq_name_to_state_map[DEFAULT_SEQUENCE_NAME] = SequenceState::Paused;
}

void AnimationManager::Stop() {
    if(m_seq_name_to_state_map.at(DEFAULT_SEQUENCE_NAME) == SequenceState::Stoped) return;

    SequenceState current_state = m_seq_name_to_state_map[DEFAULT_SEQUENCE_NAME];
    m_seq_state_to_name_map[current_state].erase(DEFAULT_SEQUENCE_NAME);
    m_seq_state_to_name_map[SequenceState::Stoped].insert(DEFAULT_SEQUENCE_NAME);
    m_seq_name_to_state_map[DEFAULT_SEQUENCE_NAME] = SequenceState::Stoped;

    for (const std::shared_ptr<TrackData>& track_data : m_sequences[DEFAULT_SEQUENCE_NAME]->data_tracks) {
        ProcessClip(DEFAULT_SEQUENCE_NAME, track_data->clip_name, 0.0f);
    }
}

void AnimationManager::Play() {
    if(m_seq_name_to_state_map.at(DEFAULT_SEQUENCE_NAME) == SequenceState::Playing) return;

    SequenceState current_state = m_seq_name_to_state_map[DEFAULT_SEQUENCE_NAME];
    m_seq_state_to_name_map[current_state].erase(DEFAULT_SEQUENCE_NAME);
    m_seq_state_to_name_map[SequenceState::Playing].insert(DEFAULT_SEQUENCE_NAME);
    m_seq_name_to_state_map[DEFAULT_SEQUENCE_NAME] = SequenceState::Playing;
}

void AnimationManager::SetClipCurrentTime(const SequenceName& seq_name, const ClipName& clip_name, float t) {
    if(!m_anim_name_to_node_map.contains(clip_name) || !m_sequences.contains(seq_name)) return;

    const std::shared_ptr<AnimationSequence>& seq = m_sequences[seq_name];

    std::vector<std::shared_ptr<TrackData>>::iterator it = std::find_if(seq->data_tracks.begin(), seq->data_tracks.end(),[&clip_name](const std::shared_ptr<TrackData>& val){return val->clip_name == clip_name;});
    if(it == seq->data_tracks.end()) return;
    
    (*it)->clip_current_time = std::fmodf(t, (*it)->clip_total_time);
}

void AnimationManager::SetSequenceCurrentTime(const SequenceName& seq_name, float t) {
    if(!m_sequences.contains(seq_name)) return;

    const std::shared_ptr<AnimationSequence>& seq = m_sequences[seq_name];
    seq->sequence_current_time = std::fmodf(t, seq->sequence_total_time);
}

float AnimationManager::GetClipCurrentTime(const SequenceName& seq_name, const ClipName& clip_name) const {
    if(!m_anim_name_to_node_map.contains(clip_name) || !m_sequences.contains(seq_name)) return;

    const std::shared_ptr<AnimationSequence>& seq = m_sequences.at(seq_name);
    std::vector<std::shared_ptr<TrackData>>::iterator it = std::find_if(seq->data_tracks.begin(), seq->data_tracks.end(),[&clip_name](const std::shared_ptr<TrackData>& val){return val->clip_name == clip_name;});
    if(it == seq->data_tracks.end()) return 0.0f;

    return (*it)->clip_current_time;
}

float AnimationManager::GetClipCurrentNormPos(const SequenceName& seq_name, const ClipName& clip_name) const {
    if(!m_anim_name_to_node_map.contains(clip_name) || !m_sequences.contains(seq_name)) return;

    const std::shared_ptr<AnimationSequence>& seq = m_sequences.at(seq_name);
    std::vector<std::shared_ptr<TrackData>>::iterator it = std::find_if(seq->data_tracks.begin(), seq->data_tracks.end(),[&clip_name](const std::shared_ptr<TrackData>& val){return val->clip_name == clip_name;});
    if(it == seq->data_tracks.end()) return 0.0f;
    
    float total_anim_time = GetClipTotalTime(clip_name);
    if(total_anim_time == 0.0f) return 0.0f;

	return (*it)->clip_current_time / total_anim_time;
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

void AnimationManager::ProcessClip(const SequenceName& seq_name, const ClipName& clip_name, float seq_t) {
    const std::shared_ptr<AnimationSequence>& seq = m_sequences.at(seq_name);
    for(const std::shared_ptr<TrackData>& track_data : seq->data_tracks) {
        if(track_data->clip_name != clip_name) continue;
        float clip_time = std::fmodf(seq_t, track_data->clip_total_time);
        for(const auto[anim_node, blend_factor] : track_data->animation_blend_factors) {
            glm::mat4x4 transform = anim_node->Get().ToParent();
            anim_node->getAnimation(clip_name)->InterpolateTime(clip_time, transform, blend_factor);
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