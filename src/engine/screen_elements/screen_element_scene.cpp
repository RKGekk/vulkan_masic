#include "screen_element_scene.h"

#include "../../application.h"
#include "../../graphics/pod/scene_config.h"
#include "../../graphics/vulkan_renderer.h"
#include "../../graphics/api/vulkan_device.h"
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
    root_node->Accept([scene](std::shared_ptr<SceneNode> node){
        std::shared_ptr<SceneNode> pMeshNode = scene->getProperty(node->VGetNodeIndex(), Scene::NODE_TYPE_FLAG_MESH);
        if(pMeshNode) {
            VulkanRenderer& renderer = Application::GetRenderer();
            std::shared_ptr<VulkanDevice> device = renderer.GetDevice();
            std::shared_ptr<MeshNode> pMeshNode = std::dynamic_pointer_cast<MeshNode>(pMeshNode);
            
            std::shared_ptr<VulkanPipeline> pipeline = renderer.getManagers()->pipelines_manager->getPipeline("basic_diffuse_pipeline"s);
            for (const std::shared_ptr<ModelData>& model_data : pMeshNode->GetMeshes()) {
                std::shared_ptr<RenderNode> render_node = std::make_shared<RenderNode>();
                render_node->init(pipeline);

                std::shared_ptr<Material> material = model_data->GetMaterial();
                std::shared_ptr<VulkanTexture> texture = material->GetTexture();
                render_node->addReadDependency(texture);
                
                std::shared_ptr<VulkanUniformBuffer> uniform_buffer = std::make_shared<VulkanUniformBuffer>(device, model_data->GetName() + "Uniform"s);
                uniform_buffer->init<UniformBufferObject>(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
                render_node->addReadDependency(uniform_buffer);

                std::shared_ptr<VertexBuffer> vertex_buffer = model_data->GetVertexBuffer();
                render_node->addReadDependency(vertex_buffer);

                std::shared_ptr<VulkanSwapChain> swap_chain_ptr = renderer.getSwapchain();
                render_node->addWriteDependency(swap_chain_ptr);

                renderer.addRenderNode(render_node);
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