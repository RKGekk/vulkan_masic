#pragma once

#include <memory>
#include <algorithm>

#include "ipointer_handler.h"
#include "ikeyboard_handler.h"

class Actor;
class TransformComponent;

class MovementController : public IPointerHandler, public IKeyboardHandler {
public:
	MovementController(std::shared_ptr<Actor> object);
	void SetObject(std::shared_ptr<Actor> new_object);

	bool VOnPointerMove(int x, int y, const int radius) override;
	bool VOnPointerButtonDown(int x, int y, const int radius, MouseButtonSide btn) override;
	bool VOnPointerButtonUp(int x, int y, const int radius, MouseButtonSide btn) override;

	bool VOnKeyDown(unsigned char c) override;
	bool VOnKeyUp(unsigned char c) override;

protected:

	std::shared_ptr<TransformComponent> gather_transform();

	int m_last_mouse_pos_x;
	int m_last_mouse_pos_y;
	bool m_bKey[256];

	bool m_mouse_LButton_down;
	bool m_mouse_RButton_down;

	std::shared_ptr<Actor> m_object;
};