#include "transform_animation_component.h"
#include "transform_component.h"

#include "../tools/string_tools.h"

const std::string TransformAnimationComponent::g_name = "TransformAnimationComponent";

TransformAnimationComponent::TransformAnimationComponent() {}

TransformAnimationComponent::TransformAnimationComponent(const pugi::xml_node& data) {
    Init(data);
}

TransformAnimationComponent::~TransformAnimationComponent() {
    
}

const std::string& TransformAnimationComponent::VGetName() const {
    return g_name;
}

const ComponentDependecyList& TransformAnimationComponent::VGetComponentDependecy() const {
    static const ComponentDependecyList component_dep = {TransformComponent::g_name};
    return component_dep;
}

pugi::xml_node TransformAnimationComponent::VGenerateXml() {
    return pugi::xml_node();
}

void TransformAnimationComponent::VPostInit() {
    std::shared_ptr<Actor> act = GetOwner();
    std::string name = act->GetName();
    
}

bool TransformAnimationComponent::VInit(const pugi::xml_node& pData) {
    return Init(pData);
}

void TransformAnimationComponent::VUpdate(const GameTimerDelta& delta) {
    std::shared_ptr<Actor> actor_ptr = GetOwner();
	std::shared_ptr<TransformComponent> tc = actor_ptr->GetComponent<TransformComponent>().lock();
		
	//glm::mat4x4 transform = tc->GetTransform();
	glm::mat4x4 transform = glm::mat4x4(1.0f);

	bool has_changes = false;
	for (auto&[anim_name, p_anim] : m_animation_map) {
        if (p_anim->AnimationState != MatrixAnimation::AnimState::Playing) continue;

		p_anim->CurrentTime.AddDeltaDuration(delta);
        float t = p_anim->CurrentTime.fGetTotalSeconds();
		p_anim->Interpolate(t, transform);

		has_changes = true;
	}

	if(has_changes) {
		tc->SetTransform(transform);
	}
}

bool TransformAnimationComponent::Init(const pugi::xml_node& data) {
    //glm::vec position = posfromattr3f(data.child("Position"));

    for (pugi::xml_node anim_node = data.first_child(); anim_node; anim_node = anim_node.next_sibling()) {
        std::string anim_name = anim_node.attribute("name").as_string();
        for (pugi::xml_node keyframe_seq_node = anim_node.first_child(); keyframe_seq_node; keyframe_seq_node = keyframe_seq_node.next_sibling()) {
            AddActorAnimation(anim_name, keyframe_seq_node);
        }
	}

	m_initialized = true;

    return m_initialized;
}

void TransformAnimationComponent::Pause() {
    for (const auto&[anim_name, anim_ptr] : m_animation_map) {
        anim_ptr->AnimationState = MatrixAnimation::AnimState::Paused;
    }
}

void TransformAnimationComponent::Pause(const MatrixAnimation::AnimationName& name) {
    if(!m_animation_map.contains(name)) return;
    m_animation_map[name]->AnimationState = MatrixAnimation::AnimState::Paused;
}

void TransformAnimationComponent::Stop() {
	for (const auto&[anim_name, anim_ptr] : m_animation_map) {
        anim_ptr->AnimationState = MatrixAnimation::AnimState::Stoped;
        anim_ptr->CurrentTime.ResetDuration();
    }
}

void TransformAnimationComponent::Stop(const MatrixAnimation::AnimationName& name) {
    if(!m_animation_map.contains(name)) return;
    m_animation_map[name]->AnimationState = MatrixAnimation::AnimState::Stoped;
    m_animation_map[name]->CurrentTime.ResetDuration();
}

void TransformAnimationComponent::Play() {
	for (const auto&[anim_name, anim_ptr] : m_animation_map) {
        anim_ptr->AnimationState = MatrixAnimation::AnimState::Playing;
    }
}

void TransformAnimationComponent::Play(const MatrixAnimation::AnimationName& name) {
    if(!m_animation_map.contains(name)) return;
    m_animation_map[name]->AnimationState = MatrixAnimation::AnimState::Playing;
}

void TransformAnimationComponent::SetCurrentAnimationTime(float t) {
    for (const auto&[anim_name, anim_ptr] : m_animation_map) {
        anim_ptr->CurrentTime.ResetDuration();
        anim_ptr->CurrentTime.AddDeltaDuration(t);
    }
}

void TransformAnimationComponent::SetCurrentAnimationTime(const MatrixAnimation::AnimationName& name, float t) {
    if(!m_animation_map.contains(name)) return;
    m_animation_map[name]->CurrentTime.ResetDuration();
    m_animation_map[name]->CurrentTime.AddDeltaDuration(t);
}

void TransformAnimationComponent::SetCurrentAnimationDuration(const GameTimerDelta& duration) {
	for (const auto&[anim_name, anim_ptr] : m_animation_map) {
        anim_ptr->CurrentTime.ResetDuration();
        anim_ptr->CurrentTime.AddDeltaDuration(duration);
    }
}

void TransformAnimationComponent::SetCurrentAnimationDuration(const MatrixAnimation::AnimationName& name, const GameTimerDelta& duration) {
    if(!m_animation_map.contains(name)) return;
    m_animation_map[name]->CurrentTime.ResetDuration();
    m_animation_map[name]->CurrentTime.AddDeltaDuration(duration);
}

float TransformAnimationComponent::GetCurrentAnimationTime(const MatrixAnimation::AnimationName& name) const {
    return m_animation_map.at(name)->CurrentTime.GetDeltaSeconds();
}

float TransformAnimationComponent::GetCurrentAnimationNormPos(const MatrixAnimation::AnimationName& name) const {
    if(!m_animation_map.contains(name)) return 0.0f;
    float total_anim_time = GetTotalAnimationTime(name);
    if(total_anim_time == 0.0f) return 0.0f;
    return m_animation_map.at(name)->CurrentTime.GetDeltaSeconds() / total_anim_time;
}

const GameTimerDelta& TransformAnimationComponent::GetCurrentAnimationDuration(const MatrixAnimation::AnimationName& name) const {
    return m_animation_map.at(name)->CurrentTime;
}

float TransformAnimationComponent::GetTotalAnimationTime(const MatrixAnimation::AnimationName& name) const {
    const std::shared_ptr<MatrixAnimation>& anim = m_animation_map.at(name);
	float t1 = anim->RotationKeyframes.size() > 0u ? anim->RotationKeyframes.back().TimePos : 0.0f;
	float t2 = anim->TranslationKeyframes.size() > 0u ? anim->TranslationKeyframes.back().TimePos : 0.0f;
    return t1 > t2 ? t1 : t2;
}

GameTimerDelta TransformAnimationComponent::GetTotalAnimationDuration(const MatrixAnimation::AnimationName& name) const {
    const std::shared_ptr<MatrixAnimation>& anim = m_animation_map.at(name);
    float t1 = anim->RotationKeyframes.size() > 0u ? anim->RotationKeyframes.back().TimePos : 0.0f;
	float t2 = anim->TranslationKeyframes.size() > 0u ? anim->TranslationKeyframes.back().TimePos : 0.0f;
	float t = t1 > t2 ? t1 : t2;
	GameTimerDelta dt;
	dt.AddDeltaDuration(t);
    return dt;
}

const std::unordered_map<MatrixAnimation::AnimationName, std::shared_ptr<MatrixAnimation>>& TransformAnimationComponent::GetAnimationMap() const {
	return m_animation_map;
}

void TransformAnimationComponent::AddActorAnimation(const MatrixAnimation::AnimationName& name, const pugi::xml_node& keyframe_seq_data) {
	m_animation_map.emplace(name, std::make_shared<MatrixAnimation>());
	std::shared_ptr<MatrixAnimation>& act_anim = m_animation_map[name];
	act_anim->Name = name;
	act_anim->AnimationState = MatrixAnimation::AnimState::Stoped;

	for(pugi::xml_node kf_node = keyframe_seq_data.first_child(); kf_node; kf_node = kf_node.next_sibling()) {

		pugi::xml_node time_node = kf_node.child("TimePosSec");
		float time_pos = time_node.text().as_float();

		pugi::xml_node trans_node = kf_node.child("Translation");
		pugi::xml_node tangs_node = kf_node.child("Tangent");
		if (trans_node) {
			KeyframeMatrixTranslation trans_frame;
			trans_frame.TimePos = time_pos;
			glm::vec3 pos = posfromattr3f(trans_node);
			glm::vec3 tg = posfromattr3f(tangs_node);
			trans_frame.Translation = pos;
			trans_frame.Tangent = tg;
			act_anim->TranslationKeyframes.push_back(trans_frame);
		}

		pugi::xml_node ypr_node = kf_node.child("YawPitchRoll");
		if (ypr_node) {
			KeyframeMatrixRotation rotation_frame;
			rotation_frame.TimePos = time_pos;
			glm::vec3 yaw_pitch_roll = anglesfromattr3f(ypr_node);
			rotation_frame.RotationQuat = glm::eulerAngleYXZ(yaw_pitch_roll.x, yaw_pitch_roll.y, yaw_pitch_roll.z);
			act_anim->RotationKeyframes.push_back(rotation_frame);
		}

		pugi::xml_node scale_node = kf_node.child("Scale");
		if (scale_node) {
			KeyframeMatrixScale scale_frame;
			scale_frame.TimePos = time_pos;
			glm::vec3 scale = posfromattr3f(scale_node);
			scale_frame.Scale = scale;
			act_anim->ScaleKeyframes.push_back(scale_frame);
		}
	}
}