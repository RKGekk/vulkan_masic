#pragma once

#include "../../events/cicadas/mouse_button_event_args.h"

#include <string>

class IPointerHandler {
public:
	virtual bool VOnPointerMove(int x, int y, const int radius) = 0;
	virtual bool VOnPointerButtonDown(int x, int y, const int radius, MouseButtonSide btn) = 0;
	virtual bool VOnPointerButtonUp(int x, int y, const int radius, MouseButtonSide btn) = 0;
};