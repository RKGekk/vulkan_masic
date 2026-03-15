#include "screen_element_scene.h"

#include "../../application.h"
#include "../../graphics/pod/scene_config.h"
#include "../../graphics/vulkan_renderer.h"
#include "../../graphics/api/vulkan_image_buffer.h"
#include "../../graphics/api/vulkan_device.h"
#include "../../graphics/drawables/scene_drawable.h"
#include "../../events/cicadas/evt_data_new_model_component.h"
#include "../../scene/nodes/scene_node.h"
#include "../../scene/nodes/mesh_node.h"
#include "../../graphics/pod/render_node.h"

ScreenElementScene::ScreenElementScene() : Scene() {
    RegisterAllDelegates();
};

ScreenElementScene::~ScreenElementScene() {
    RemoveAllDelegates();
};

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

void ScreenElementScene::ModifiedSceneNodeComponentDelegate(IEventDataPtr pEventData) {};

void ScreenElementScene::NewModelComponentDelegate(IEventDataPtr pEventData) {
    std::shared_ptr<EvtData_New_Model_Component> pCastEventData = std::static_pointer_cast<EvtData_New_Model_Component>(pEventData);
	std::shared_ptr<SceneNode> node = pCastEventData->GetSceneNode().lock();

	NewModelComponent(node);
}

void ScreenElementScene::ModifiedSceneNode(std::shared_ptr<SceneNode> node) {};

void ScreenElementScene::NewModelComponent(std::shared_ptr<SceneNode> root_node) {
    using namespace std::literals;

    std::shared_ptr<Scene> scene = root_node->GetScene();
    root_node->Accept([scene, drawable = m_scene_draw](std::shared_ptr<SceneNode> node){
        std::shared_ptr<SceneNode> pMeshNode = scene->getProperty(node->VGetNodeIndex(), Scene::NODE_TYPE_FLAG_MESH);
        if(pMeshNode) {
            std::shared_ptr<SceneNode> pMeshNode = scene->getProperty(node->VGetNodeIndex(), Scene::NODE_TYPE_FLAG_MESH);
            if(pMeshNode) {
                std::shared_ptr<MeshNode> pMesh = std::dynamic_pointer_cast<MeshNode>(pMeshNode);
                drawable->addRendeNode(pMesh, Application::GetRenderer().getManagers());
            }
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