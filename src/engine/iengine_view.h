#pragma once

#include <deque>
#include <memory>
#include <string>

#include "engine_view_type.h"
#include "../actors/actor.h"
#include "../tools/game_timer.h"

class IEngineView;

typedef unsigned int EngineViewId;
typedef std::deque<std::shared_ptr<IEngineView>> GameViewList;

class IEngineView {
public:
	virtual bool VOnRestore() = 0;
	virtual bool VOnLostDevice() = 0;

	virtual void VOnUpdate(const GameTimerDelta& delta) = 0;
	
	virtual EngineViewType VGetType() = 0;
	virtual const std::string& VGetName() = 0;
	virtual EngineViewId VGetId() const = 0;

	virtual void VOnAttach(EngineViewId vid, ActorId aid) = 0;

	virtual ~IEngineView() {};
};