#include "base_scene_node_component.h"

#include "../tools/string_tools.h"
#include "transform_component.h"
#include "../events/cicadas/evt_data_new_scene_component.h"
#include "../events/cicadas/evt_data_destroy_scene_component.h"
#include "../events/cicadas/evt_data_modified_scene_component.h"
#include "../events/ievent_manager.h"

BaseSceneNodeComponent::~BaseSceneNodeComponent() {}

void BaseSceneNodeComponent::VPostInit() {
	std::shared_ptr<Actor> act = GetOwner();
	std::string name = act->GetName();
	std::shared_ptr<SceneNode> scene_node = VGetSceneNode();
	scene_node->SetName(name);
	std::shared_ptr<TransformComponent> tc = act->GetComponent<TransformComponent>(ActorComponent::GetIdFromName("TransformComponent")).lock();
	if (tc) {
		scene_node->SetTransform(tc->GetTransform());
	}
	VDelegatePostInit();
	std::shared_ptr<EvtData_New_Scene_Component> pNewSceneNodeEvent = std::make_shared<EvtData_New_Scene_Component>(act->GetId(), VGetId(), scene_node);
	IEventManager::Get()->VQueueEvent(pNewSceneNodeEvent);
}