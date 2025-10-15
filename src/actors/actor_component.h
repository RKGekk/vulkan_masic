#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>


#include <pugixml.hpp>

#include "actor.h"
#include "../tools/game_timer.h"

using ComponentDependecyList = std::vector<std::string>;

class ActorComponent {
	friend class ActorFactory;

public:
	virtual ~ActorComponent();

	virtual bool VInit(const pugi::xml_node& data) = 0;
	virtual void VPostInit();
	virtual void VUpdate(const GameTimerDelta& delta);
	virtual void VOnChanged();

	virtual pugi::xml_node VGenerateXml() = 0;

	virtual ComponentId VGetId() const;
	virtual const std::string& VGetName() const = 0;
	virtual const ComponentDependecyList& VGetComponentDependecy() const = 0;

	void SetOwner(StrongActorPtr pOwner);
	StrongActorPtr GetOwner();
	ActorId GetOwnerId();
	bool GetIsInitialized();

	static ComponentId GetIdFromName(const std::string& componentStr);

protected:
	virtual void VRegisterEvents() = 0;

	inline static bool m_events_registered = false;
	bool m_initialized = false;
	StrongActorPtr m_pOwner;
};