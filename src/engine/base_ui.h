#pragma once

#include <optional>

#include "iscreen_element.h"

class BaseUI : public IScreenElement {
public:
	BaseUI();
	virtual bool VOnLostDevice() override;
	virtual void VOnUpdate(const GameTimerDelta& delta) override;

	virtual bool VIsVisible() const override;
	virtual void VSetVisible(bool visible) override;

protected:
	int m_pos_x;
	int m_pos_y;
	int m_width;
	int m_height;
	bool m_is_visible;

	std::optional<int> m_result;
};