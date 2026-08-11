#pragma once

#include <string>
#include <memory>

#include "base_ui.h"
#include "../../scene/animation_manager.h"

class BaseEngineLogic;

class ManagersMenuUI : public BaseUI {
public:
	ManagersMenuUI();
	virtual ~ManagersMenuUI();

	virtual bool VOnRestore() override;
	virtual bool VOnRender(const GameTimerDelta& delta, uint32_t image_index) override;
	virtual void VOnUpdate(const GameTimerDelta& delta, uint32_t image_index) override;

	virtual int VGetZOrder() const override;
	virtual void VSetZOrder(int const zOrder) override;

private:
    //std::shared_ptr<AnimationManager> m_animation_manager;
};