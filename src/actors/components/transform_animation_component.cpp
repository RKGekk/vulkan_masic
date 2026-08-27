#include "transform_animation_component.h"
#include "transform_component.h"

#include "../../tools/string_tools.h"

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
    // std::shared_ptr<Actor> act = GetOwner();
    // std::string name = act->GetName();
}

bool TransformAnimationComponent::VInit(const pugi::xml_node& pData) {
	std::shared_ptr<Actor> actor_ptr = GetOwner();
	std::shared_ptr<TransformComponent> tc = actor_ptr->GetComponent<TransformComponent>().lock();
	std::shared_ptr<Scene> scene = tc->GetSceneNode()->GetScene();
	Scene::NodeIndex node_idx = tc->GetSceneNode()->VGetNodeIndex();

	if(scene->getNodeTypeFlags(node_idx) && Scene::NODE_TYPE_FLAG_ANIMATION) {
		m_animation_node = std::dynamic_pointer_cast<AnimationNode>(scene->getProperty(node_idx, Scene::NODE_TYPE_FLAG_ANIMATION));
	}
	else {
		m_animation_node = std::make_shared<AnimationNode>(scene, node_idx);
		scene->addProperty(m_animation_node);
	}

    return Init(pData);
}

void TransformAnimationComponent::VUpdate(const GameTimerDelta& delta) {
    std::shared_ptr<Actor> actor_ptr = GetOwner();
	std::shared_ptr<TransformComponent> tc = actor_ptr->GetComponent<TransformComponent>().lock();
		
	//glm::mat4x4 transform = tc->GetTransform();
	glm::mat4x4 transform = glm::mat4x4(1.0f);

	bool has_changes = false;
	for (auto&[anim_name, anim_data] : m_animation_data_map) {
        if (anim_data.animation_state != AnimState::Playing) continue;

		const std::shared_ptr<MatrixAnimation>& p_anim = m_animation_node->getAnimation(anim_name);
		anim_data.current_time.AddDeltaDuration(delta);
		p_anim->InterpolateTime(anim_data.current_time.fGetTotalSeconds(), transform);

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
    for (auto&[anim_name, anim_data] : m_animation_data_map) {
        anim_data.animation_state = AnimState::Paused;
    }
}

void TransformAnimationComponent::Pause(const AnimationNode::AnimationName& name) {
    if(!m_animation_data_map.contains(name)) return;
    m_animation_data_map[name].animation_state = AnimState::Paused;
}

void TransformAnimationComponent::Stop() {
	for (auto&[anim_name, anim_data] : m_animation_data_map) {
        anim_data.animation_state = AnimState::Stoped;
        anim_data.current_time.ResetDuration();
    }
}

void TransformAnimationComponent::Stop(const AnimationNode::AnimationName& name) {
    if(!m_animation_data_map.contains(name)) return;
    m_animation_data_map[name].animation_state = AnimState::Stoped;
    m_animation_data_map[name].current_time.ResetDuration();

	std::shared_ptr<Actor> actor_ptr = GetOwner();
	std::shared_ptr<TransformComponent> tc = actor_ptr->GetComponent<TransformComponent>().lock();

	//glm::mat4x4 transform = tc->GetTransform();
	glm::mat4x4 transform = glm::mat4x4(1.0f);

	const std::shared_ptr<MatrixAnimation>& p_anim = m_animation_node->getAnimation(name);
	p_anim->InterpolateTime(0, transform);

	tc->SetTransform(transform);
}

void TransformAnimationComponent::Play() {
	for (auto&[anim_name, anim_data] : m_animation_data_map) {
        anim_data.animation_state = AnimState::Playing;
    }
}

void TransformAnimationComponent::Play(const AnimationNode::AnimationName& name) {
    if(!m_animation_data_map.contains(name)) return;
    m_animation_data_map[name].animation_state = AnimState::Playing;
}

void TransformAnimationComponent::SetCurrentAnimationTime(float t) {
	GameTimerDelta dt;
	dt.AddDeltaDuration(t);
    for (auto&[anim_name, anim_data] : m_animation_data_map) {
        SetCurrentAnimationDuration(anim_name, dt);
    }
}

void TransformAnimationComponent::SetCurrentAnimationTime(const AnimationNode::AnimationName& name, float t) {
	GameTimerDelta dt;
	dt.AddDeltaDuration(t);
    SetCurrentAnimationDuration(name, dt);
}

void TransformAnimationComponent::SetCurrentAnimationDuration(const GameTimerDelta& duration) {
	for (auto&[anim_name, anim_data] : m_animation_data_map) {
        SetCurrentAnimationDuration(anim_name, duration);
    }
}

void TransformAnimationComponent::SetCurrentAnimationDuration(const AnimationNode::AnimationName& name, const GameTimerDelta& duration) {
    if(!m_animation_data_map.contains(name)) return;
    m_animation_data_map[name].current_time.ResetDuration();
    m_animation_data_map[name].current_time.AddDeltaDuration(duration);

    if (m_animation_data_map[name].animation_state == AnimState::Stoped) return;

	std::shared_ptr<Actor> actor_ptr = GetOwner();
	std::shared_ptr<TransformComponent> tc = actor_ptr->GetComponent<TransformComponent>().lock();

	//glm::mat4x4 transform = tc->GetTransform();
	glm::mat4x4 transform = glm::mat4x4(1.0f);

	const std::shared_ptr<MatrixAnimation>& p_anim = m_animation_node->getAnimation(name);
	p_anim->InterpolateTime(duration.fGetTotalSeconds(), transform);

	tc->SetTransform(transform);
}

float TransformAnimationComponent::GetCurrentAnimationTime(const AnimationNode::AnimationName& name) const {
    //return m_animation_map.at(name)->CurrentTime.GetDeltaSeconds();
	return m_animation_data_map.at(name).current_time.GetTotalSeconds();
}

float TransformAnimationComponent::GetCurrentAnimationNormPos(const AnimationNode::AnimationName& name) const {
    if(!m_animation_data_map.contains(name)) return 0.0f;
    float total_anim_time = GetTotalAnimationTime(name);
    if(total_anim_time == 0.0f) return 0.0f;
    //return m_animation_map.at(name)->CurrentTime.GetDeltaSeconds() / total_anim_time;
	return m_animation_data_map.at(name).current_time.GetTotalSeconds() / total_anim_time;
}

const GameTimerDelta& TransformAnimationComponent::GetCurrentAnimationDuration(const AnimationNode::AnimationName& name) const {
    return m_animation_data_map.at(name).current_time;
}

float TransformAnimationComponent::GetTotalAnimationTime(const AnimationNode::AnimationName& name) const {
    const std::shared_ptr<MatrixAnimation>& anim = m_animation_node->getAnimation(name);
	float t1 = anim->RotationKeyframes.size() > 0u ? anim->RotationKeyframes.back().TimePos : 0.0f;
	float t2 = anim->TranslationKeyframes.size() > 0u ? anim->TranslationKeyframes.back().TimePos : 0.0f;
    return t1 > t2 ? t1 : t2;
}

GameTimerDelta TransformAnimationComponent::GetTotalAnimationDuration(const AnimationNode::AnimationName& name) const {
    const std::shared_ptr<MatrixAnimation>& anim = m_animation_node->getAnimation(name);
    float t1 = anim->RotationKeyframes.size() > 0u ? anim->RotationKeyframes.back().TimePos : 0.0f;
	float t2 = anim->TranslationKeyframes.size() > 0u ? anim->TranslationKeyframes.back().TimePos : 0.0f;
	float t = t1 > t2 ? t1 : t2;
	GameTimerDelta dt;
	dt.AddDeltaDuration(t);
    return dt;
}

const std::unordered_map<AnimationNode::AnimationName, std::shared_ptr<MatrixAnimation>>& TransformAnimationComponent::GetAnimationMap() const {
	return m_animation_node->getAnimationMap();
}

void TransformAnimationComponent::AddActorAnimation(const AnimationNode::AnimationName& name, const pugi::xml_node& keyframe_seq_data) {
	std::shared_ptr<MatrixAnimation> matrix_animation = std::make_shared<MatrixAnimation>();
	

	AnimData& anim_data = m_animation_data_map[name];
	anim_data.name = name;
	anim_data.animation_state = AnimState::Stoped;
	anim_data.current_time = GameTimerDelta();

	for(pugi::xml_node kf_node = keyframe_seq_data.first_child(); kf_node; kf_node = kf_node.next_sibling()) {

		pugi::xml_node time_node = kf_node.child("TimePosSec");
		float time_pos = time_node.text().as_float();

		pugi::xml_node trans_node = kf_node.child("Translation");
		pugi::xml_node in_tg_node = kf_node.child("inTangent");
		pugi::xml_node out_tg_node = kf_node.child("outTangent");
		if (trans_node) {
			KeyframeMatrixTranslation trans_frame;
			trans_frame.TimePos = time_pos;
			glm::vec3 pos = posfromattr3f(trans_node);
			glm::vec3 in_tg = posfromattr3f(in_tg_node);
			glm::vec3 out_tg = posfromattr3f(out_tg_node);
			trans_frame.Translation = pos;
			trans_frame.inTangent = in_tg;
			trans_frame.outTangent = out_tg;
			trans_frame.InterpolationType = KeyFrameInterpolationType::CUBICSPLINE;
			matrix_animation->TranslationKeyframes.push_back(trans_frame);
		}

		pugi::xml_node ypr_node = kf_node.child("YawPitchRoll");
		if (ypr_node) {
			KeyframeMatrixRotation rotation_frame;
			rotation_frame.TimePos = time_pos;
			glm::vec3 yaw_pitch_roll = anglesfromattr3f(ypr_node);
			rotation_frame.RotationQuat = glm::eulerAngleYXZ(yaw_pitch_roll.x, yaw_pitch_roll.y, yaw_pitch_roll.z);
			rotation_frame.InterpolationType = KeyFrameInterpolationType::LINEAR;
			matrix_animation->RotationKeyframes.push_back(rotation_frame);
		}

		// pugi::xml_node scale_node = kf_node.child("Scale");
		// if (scale_node) {
		// 	KeyframeMatrixScale scale_frame;
		// 	scale_frame.TimePos = time_pos;
		// 	glm::vec3 scale = posfromattr3f(scale_node);
		// 	scale_frame.Scale = scale;
		// 	act_anim->ScaleKeyframes.push_back(scale_frame);
		// }
	}

	m_animation_node->addAnimation(name, std::move(matrix_animation));
}