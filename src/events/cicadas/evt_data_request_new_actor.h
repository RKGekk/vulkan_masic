#pragma once

#include <iostream>
#include <iomanip>

#include "../base_event_data.h"
#include "../../actors/actor.h"
#include "../../engine/views/iengine_view.h"

class EvtData_Request_New_Actor : public BaseEventData {
	std::string m_actor_name;
	ActorId m_serverActorId;

public:
	static const EventTypeId sk_EventType = 0x40378c64;
	static const std::string sk_EventName;

	EvtData_Request_New_Actor();
	explicit EvtData_Request_New_Actor(std::string actor_name, const ActorId serverActorId = 0);

	virtual EventTypeId VGetEventType() const override;
	virtual void VDeserialize(std::istream& in) override;
	virtual IEventDataPtr VCopy() const override;
	virtual void VSerialize(std::ostream& out) const override;
	virtual const std::string& GetName() const override;

	const std::string& GetActorName() const;
	const ActorId GetServerActorId() const;

	friend std::ostream& operator<<(std::ostream& os, const EvtData_Request_New_Actor& evt);
};