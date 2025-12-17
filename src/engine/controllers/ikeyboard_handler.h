#pragma once

#include "../../events/cicadas/window_keycodes.h"

class IKeyboardHandler {
public:
	virtual bool VOnKeyDown(WindowKey key, unsigned char c) = 0;
	virtual bool VOnKeyUp(WindowKey key, unsigned char c) = 0;
};