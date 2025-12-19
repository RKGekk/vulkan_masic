#pragma once

#include <string>
#include <memory>

#include "base_ui.h"

class ActorMenuUI : public BaseUI {
public:
	ActorMenuUI();
	virtual ~ActorMenuUI();

	virtual bool VOnRestore() override;
	virtual bool VOnRender(const GameTimerDelta& delta) override;
	virtual void VOnUpdate(const GameTimerDelta& delta) override;

	virtual int VGetZOrder() const override;
	virtual void VSetZOrder(int const zOrder) override;

private:
    int m_actor_id;
    std::string m_actor_name;
};