#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include "actor_component.h"
#include "../scene/nodes/scene_node.h"
#include "../tools/game_timer.h"

class BaseSceneNodeComponent : public ActorComponent {
public:
	virtual ~BaseSceneNodeComponent();

	virtual std::shared_ptr<SceneNode> VGetSceneNode() = 0;
	
	virtual void VPostInit() override;
    virtual void VDelegatePostInit() {};
};