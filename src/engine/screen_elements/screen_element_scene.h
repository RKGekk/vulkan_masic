#pragma once

#include <memory>

#include "iscreen_element.h"
#include "../../scene/scene.h"
#include "../../events/ievent_data.h"

class ScreenElementScene : public IScreenElement, public Scene {
public:
	ScreenElementScene() {};
	virtual ~ScreenElementScene() {};

	virtual bool VOnRestore() override { return true; };
	virtual bool VOnLostDevice() override { return true; };

	virtual void VOnUpdate(const GameTimerDelta& delta) override {};
	virtual bool VOnRender(const GameTimerDelta& delta) override { return true; };
	
	virtual int VGetZOrder() const override { return 1; };
	virtual void VSetZOrder(int const zOrder) override {};

	virtual bool VIsVisible() const override { return true; };
	virtual void VSetVisible(bool visible) override {};

	virtual bool VAddChild(std::shared_ptr<SceneNode> kid) { return true; };

	void ModifiedSceneNodeComponentDelegate(IEventDataPtr pEventData) {};

protected:
	void ModifiedSceneNode(std::shared_ptr<SceneNode> node) {};

private:
	bool m_is_visible = true;
	uint32_t m_width;
	uint32_t m_height;

private:
	void RegisterAllDelegates() {};
	void RemoveAllDelegates() {};
};