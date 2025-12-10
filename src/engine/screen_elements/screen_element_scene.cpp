#include "screen_element_scene.h"

#include "../../application.h"
#include "../../graphics/scene_config.h"
#include "../../graphics/vulkan_renderer.h"
#include "../../graphics/vulkan_device.h"
#include "../../events/cicadas/evt_data_new_model_component.h"
#include "../../scene/nodes/scene_node.h"
#include "../../scene/nodes/mesh_node.h"

ScreenElementScene::ScreenElementScene() : Scene() {
    VulkanRenderer& renderer = Application::GetRenderer();
	std::shared_ptr<VulkanDevice> device = renderer.GetDevice();

    m_scene_draw = std::make_shared<SceneDrawable>();
    m_scene_draw->init(device, renderer.getRenderTarget(), renderer.getSwapchain()->getMaxFrames());
	renderer.addDrawable(m_scene_draw);
};

ScreenElementScene::~ScreenElementScene() {};

bool ScreenElementScene::VOnRestore() {
    return true;
};

bool ScreenElementScene::VOnLostDevice() {
    return true;
};

void ScreenElementScene::VOnUpdate(const GameTimerDelta& delta) {};

bool ScreenElementScene::VOnRender(const GameTimerDelta& delta) {
    return true;
};
	
int ScreenElementScene::VGetZOrder() const {
    return 1;
};

void ScreenElementScene::VSetZOrder(int const zOrder) {};

bool ScreenElementScene::VIsVisible() const {
    return true;
};

void ScreenElementScene::VSetVisible(bool visible) {

};

bool ScreenElementScene::VAddChild(std::shared_ptr<SceneNode> kid) {
    return true;
};

void ScreenElementScene::ModifiedSceneNodeComponentDelegate(IEventDataPtr pEventData) {};

void ScreenElementScene::NewModelComponentDelegate(IEventDataPtr pEventData) {
    std::shared_ptr<EvtData_New_Model_Component> pCastEventData = std::static_pointer_cast<EvtData_New_Model_Component>(pEventData);
	std::shared_ptr<SceneNode> node = pCastEventData->GetSceneNode().lock();

	NewModelComponent(node);
}

void ScreenElementScene::ModifiedSceneNode(std::shared_ptr<SceneNode> node) {};

void ScreenElementScene::NewModelComponent(std::shared_ptr<SceneNode> root_node) {
    root_node->Accept([drawable = m_scene_draw](std::shared_ptr<SceneNode> node){
        std::shared_ptr<MeshNode> pMeshNode = std::dynamic_pointer_cast<MeshNode>(node);
        if(pMeshNode) {
            drawable->addRendeNode(pMeshNode);
        }
    });
}

void ScreenElementScene::RegisterAllDelegates() {
    IEventManager* pGlobalEventManager = IEventManager::Get();
	pGlobalEventManager->VAddListener({ connect_arg<&ScreenElementScene::NewModelComponentDelegate>, this }, EvtData_New_Model_Component::sk_EventType);
};

void ScreenElementScene::RemoveAllDelegates() {
    IEventManager* pGlobalEventManager = IEventManager::Get();
	pGlobalEventManager->VRemoveListener({ connect_arg<&ScreenElementScene::NewModelComponentDelegate>, this }, EvtData_New_Model_Component::sk_EventType);
};