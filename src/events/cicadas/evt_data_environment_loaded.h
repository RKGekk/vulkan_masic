#pragma once

#include <iostream>
#include <iomanip>

#include "../base_event_data.h"
#include "../../tools/string_tools.h"

class EvtData_Environment_Loaded : public BaseEventData {
public:
	inline static const EventTypeId sk_EventType = 0x8E2AD6E6;
	//inline static const std::string sk_EventName = "EvtData_Environment_Loaded";

	EvtData_Environment_Loaded();

	virtual EventTypeId VGetEventType() const override;
	virtual IEventDataPtr VCopy() const override;
	//virtual const std::string& GetName() const override;

	friend std::ostream& operator<<(std::ostream& os, const EvtData_Environment_Loaded& evt);
};