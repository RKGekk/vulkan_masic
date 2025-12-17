#pragma once

#include <memory>
#include <algorithm>

#include "imovement_controller.h"
#include "ipointer_handler.h"
#include "ikeyboard_handler.h"

class Actor;
class TransformComponent;

class MovementController : public IPointerHandler, public IKeyboardHandler, public IMovementController {
public:
	MovementController(std::shared_ptr<Actor> object);

	virtual void SetObject(std::shared_ptr<Actor> new_object) override;
	virtual void OnUpdate(const GameTimerDelta& delta) override;

	bool VOnPointerMove(int x, int y, const int radius) override;
	bool VOnPointerButtonDown(int x, int y, const int radius, MouseButtonSide btn) override;
	bool VOnPointerButtonUp(int x, int y, const int radius, MouseButtonSide btn) override;

	bool VOnKeyDown(WindowKey key, unsigned char c) override;
	bool VOnKeyUp(WindowKey key, unsigned char c) override;

protected:

	std::shared_ptr<TransformComponent> gather_transform();

	int m_last_mouse_pos_x;
	int m_last_mouse_pos_y;
	bool m_key[256];
	bool m_char[256];

	bool m_mouse_LButton_down;
	bool m_mouse_RButton_down;

	std::shared_ptr<Actor> m_object;
};