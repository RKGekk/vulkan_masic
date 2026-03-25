#pragma once

#include <string>
#include <memory>

#include "base_ui.h"

class TestMenuUI : public BaseUI {
public:
	TestMenuUI();
	virtual ~TestMenuUI();

	virtual bool VOnRestore() override;
	virtual bool VOnRender(const GameTimerDelta& delta, uint32_t image_index) override;
	virtual void VOnUpdate(const GameTimerDelta& delta, uint32_t image_index) override;

	virtual int VGetZOrder() const override;
	virtual void VSetZOrder(int const zOrder) override;
};