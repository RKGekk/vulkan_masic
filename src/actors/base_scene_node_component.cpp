#include "base_scene_node_component.h"

#include "../tools/string_tools.h"
#include "transform_component.h"
#include "../events/cicadas/evt_data_new_scene_component.h"
#include "../events/cicadas/evt_data_destroy_scene_component.h"
#include "../events/cicadas/evt_data_modified_scene_component.h"
#include "../events/ievent_manager.h"

BaseSceneNodeComponent::~BaseSceneNodeComponent() {}

void BaseSceneNodeComponent::VPostInit() {
	VDelegatePostInit();
}