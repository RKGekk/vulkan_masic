#pragma once

#include <memory>

#include "iscreen_element.h"
#include "../scene/scene.h"
#include "../events/ievent_data.h"

class ScreenElementScene : public IScreenElement, public Scene {
public:
	ScreenElementScene();
	virtual ~ScreenElementScene();

	virtual bool VOnRestore() override;
	virtual bool VOnLostDevice() override;

	virtual void VOnUpdate(const GameTimerDelta& delta) override;
	virtual bool VOnRender(const GameTimerDelta& delta, std::shared_ptr<CommandList> command_list) override;
	
	virtual int VGetZOrder() const override;
	virtual void VSetZOrder(int const zOrder) override;

	virtual bool VIsVisible() const override;
	virtual void VSetVisible(bool visible) override;

	virtual bool VAddChild(std::shared_ptr<SceneNode> kid);

	void ModifiedSceneNodeComponentDelegate(IEventDataPtr pEventData);

protected:
	void ModifiedSceneNode(std::shared_ptr<SceneNode> node);

private:
	bool m_is_visible = true;
	uint32_t m_width;
	uint32_t m_height;

private:
	void RegisterAllDelegates();
	void RemoveAllDelegates();
};