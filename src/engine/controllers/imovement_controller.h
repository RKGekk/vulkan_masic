#pragma once

#include <memory>

#include "ipointer_handler.h"
#include "ikeyboard_handler.h"
#include "../../tools/game_timer.h"

class Actor;

class IMovementController {
public:
	virtual void SetObject(std::shared_ptr<Actor> new_object) = 0;
	virtual void OnUpdate(const GameTimerDelta& delta) = 0;
};