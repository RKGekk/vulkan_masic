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
    void Pause(const MatrixAnimation::AnimationName& name);

	void Stop();
    void Stop(const MatrixAnimation::AnimationName& name);

	void Play();
    void Play(const MatrixAnimation::AnimationName& name);

    void SetCurrentAnimationTime(float t);
	void SetCurrentAnimationTime(const MatrixAnimation::AnimationName& name, float t);
    void SetCurrentAnimationDuration(const GameTimerDelta& duration);
	void SetCurrentAnimationDuration(const MatrixAnimation::AnimationName& name, const GameTimerDelta& duration);
    
    float GetCurrentAnimationTime(const MatrixAnimation::AnimationName& name) const;
    float GetCurrentAnimationNormPos(const MatrixAnimation::AnimationName& name) const;
    const GameTimerDelta& GetCurrentAnimationDuration(const MatrixAnimation::AnimationName& name) const;
    float GetTotalAnimationTime(const MatrixAnimation::AnimationName& name) const;
    GameTimerDelta GetTotalAnimationDuration(const MatrixAnimation::AnimationName& name) const;

    const std::unordered_map<MatrixAnimation::AnimationName, std::shared_ptr<MatrixAnimation>>& GetAnimationMap() const;

private:
    void AddActorAnimation(const MatrixAnimation::AnimationName& name, const pugi::xml_node& keyframe_seq_data);

    bool Init(const pugi::xml_node& data);

    std::unordered_map<MatrixAnimation::AnimationName, std::shared_ptr<MatrixAnimation>> m_animation_map;
};