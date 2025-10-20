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

#include <iostream>
#include <iomanip>

#include "../base_event_data.h"
#include "../../actors/actor.h"
#include "../../tools/string_tools.h"

class EvtData_Move_Actor : public BaseEventData {
    ActorId m_id;
    glm::mat4x4 m_matrix;

public:
    static const EventTypeId sk_EventType = 0xeeaa0a40;
    static const std::string sk_EventName;

    EvtData_Move_Actor();
    EvtData_Move_Actor(ActorId id, const glm::mat4x4& matrix);

    virtual EventTypeId VGetEventType() const override;
    virtual void VSerialize(std::ostream& out) const override;
    virtual void VDeserialize(std::istream& in) override;
    virtual IEventDataPtr VCopy() const override;
    virtual const std::string& GetName() const override;

    ActorId GetId() const;
    const glm::mat4x4& GetMatrix() const;

    friend std::ostream& operator<<(std::ostream& os, const EvtData_Move_Actor& evt);
};