#pragma once

#include <string>
#include <memory>

#include "base_ui.h"

class Scene;

class NodeMenuUI : public BaseUI {
public:
	NodeMenuUI();
	virtual ~NodeMenuUI();

	virtual bool VOnRestore() override;
	virtual bool VOnRender(const GameTimerDelta& delta) override;
	virtual void VOnUpdate(const GameTimerDelta& delta) override;

	virtual int VGetZOrder() const override;
	virtual void VSetZOrder(int const zOrder) override;

private:
    
};