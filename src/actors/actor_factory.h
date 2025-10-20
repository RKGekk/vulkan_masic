#pragma once

#include <map>
#include <string>
#include <memory>
#include <unordered_map>
#include <vector>

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>
#include "glm/gtc/quaternion.hpp"
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <pugixml.hpp>

#include "Actor.h"
#include "actor_component.h"
#include "../tools/generic_object_factory.h"
#include "../tools/memory_utility.h"

class ActorFactory {
    ActorId m_last_actorId;

protected:
    GenericObjectFactory<ActorComponent, ComponentId> m_component_factory;

public:
    ActorFactory();

    std::shared_ptr<Actor> CreateActor(const std::string& actor_resource, const pugi::xml_node& overrides, const ActorId servers_actorId);
    void ModifyActor(std::shared_ptr<Actor> pActor, const pugi::xml_node& overrides);

    virtual std::shared_ptr<ActorComponent> VCreateComponent(const pugi::xml_node& pData);

private:
    std::unordered_map<std::string, std::pair<std::shared_ptr<ActorComponent>, pugi::xml_node>> getAllComponents(pugi::xml_node actor_node);
    ActorId GetNextActorId();
};