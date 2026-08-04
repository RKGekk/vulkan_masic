#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

#include "actor_component.h"
#include "../animation/matrix_animation.h"
#include "../scene/nodes/animation_node.h"

#include <pugixml.hpp>

#include <memory>
#include <string>
#include <vector>

class TransformAnimationComponent : public ActorComponent {
public:
    enum class AnimState {
		Playing,
		Stoped,
        Paused
	};

    struct AnimData {
        AnimationNode::AnimationName name;
        AnimState animation_state;
	    GameTimerDelta current_time;
    };

    static const std::string g_name;

    TransformAnimationComponent();
    TransformAnimationComponent(const pugi::xml_node& data);
    virtual ~TransformAnimationComponent();

    virtual bool VInit(const pugi::xml_node& data) override;
    virtual const std::string& VGetName() const override;
    virtual const ComponentDependecyList& VGetComponentDependecy() const override;
    virtual pugi::xml_node VGenerateXml() override;
    virtual void VPostInit() override;
    virtual void VUpdate(const GameTimerDelta& delta) override;

    void Pause();
    void Pause(const AnimationNode::AnimationName& name);

	void Stop();
    void Stop(const AnimationNode::AnimationName& name);

	void Play();
    void Play(const AnimationNode::AnimationName& name);

    void SetCurrentAnimationTime(float t);
	void SetCurrentAnimationTime(const AnimationNode::AnimationName& name, float t);
    void SetCurrentAnimationDuration(const GameTimerDelta& duration);
	void SetCurrentAnimationDuration(const AnimationNode::AnimationName& name, const GameTimerDelta& duration);
    
    float GetCurrentAnimationTime(const AnimationNode::AnimationName& name) const;
    float GetCurrentAnimationNormPos(const AnimationNode::AnimationName& name) const;
    const GameTimerDelta& GetCurrentAnimationDuration(const AnimationNode::AnimationName& name) const;
    float GetTotalAnimationTime(const AnimationNode::AnimationName& name) const;
    GameTimerDelta GetTotalAnimationDuration(const AnimationNode::AnimationName& name) const;

    const std::unordered_map<AnimationNode::AnimationName, std::shared_ptr<MatrixAnimation>>& GetAnimationMap() const;

private:
    void AddActorAnimation(const AnimationNode::AnimationName& name, const pugi::xml_node& keyframe_seq_data);

    bool Init(const pugi::xml_node& data);

    std::shared_ptr<AnimationNode> m_animation_node;
    std::unordered_map<AnimationNode::AnimationName, AnimData> m_animation_data_map;
};