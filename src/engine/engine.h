#pragma once

#include <memory>

#include "../application_options.h"
#include "base_engine_logic.h"
#include "../graphics/vulkan_renderer.h"

class Engine {
public:
	virtual ~Engine() {};
	static void Destroy() {};

	bool Initialize(ApplicationOptions opt) { return true; };

	std::shared_ptr<BaseEngineLogic> GetGameLogic() { return nullptr; };
	std::shared_ptr<VulkanRenderer> GetRenderer() { return nullptr; };
	static std::shared_ptr<Engine> GetEngine() { return nullptr; };

	void Update(IEventDataPtr pEventData) {};
	void RenderFrame() {};

protected:
	Engine();
	virtual std::shared_ptr<BaseEngineLogic> VCreateGameAndView() { return nullptr; };
	virtual void VRegisterEvents() {};
	virtual void RegisterAllDelegates() {};

private:

	ApplicationOptions m_options;

	std::shared_ptr<VulkanRenderer> m_renderer;
	std::shared_ptr<BaseEngineLogic> m_game;

	static std::shared_ptr<Engine> m_pEngine;
};