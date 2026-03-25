#pragma once

#include <string>
#include <memory>

#include "base_ui.h"

class BaseEngineLogic;

class ActorMenuUI : public BaseUI {
public:
	ActorMenuUI();
	virtual ~ActorMenuUI();

	virtual bool VOnRestore() override;
	virtual bool VOnRender(const GameTimerDelta& delta, uint32_t image_index) override;
	virtual void VOnUpdate(const GameTimerDelta& delta, uint32_t image_index) override;

	virtual int VGetZOrder() const override;
	virtual void VSetZOrder(int const zOrder) override;

private:
    int m_actor_id;
};