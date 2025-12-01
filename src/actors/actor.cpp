#include "actor.h"

#include "actor_component.h"
#include "../events/cicadas/evt_data_new_actor.h"
#include "../events/cicadas/evt_data_destroy_actor.h"
#include "../events/ievent_manager.h"

Actor::Actor(ActorId id) {
    m_id = id;
    m_name = "NoName";
}

Actor::~Actor() {
    std::shared_ptr<EvtData_Destroy_Actor> pDestroyActorEvent = std::make_shared<EvtData_Destroy_Actor>(GetId());
    IEventManager::Get()->VQueueEvent(pDestroyActorEvent);
}

bool Actor::Init(const pugi::xml_node& data) {
    m_name = data.attribute("name").value();
    return true;
}

void Actor::PostInit() {
    for (auto it = m_components.begin(); it != m_components.end(); ++it) {
        it->second->VPostInit();
    }
}

void Actor::Destroy() {
    m_components.clear();
}

void Actor::Update(const GameTimerDelta& delta) {
    for (auto it = m_components.begin(); it != m_components.end(); ++it) {
        it->second->VUpdate(delta);
    }
}

ActorId Actor::GetId() const {
    return m_id;
}

const std::string& Actor::GetName() const {
    return m_name;
}

void Actor::SetName(std::string new_name) {
    m_name = std::move(new_name);
}

const ActorComponents& Actor::GetComponents() {
    return m_components;
}

void Actor::AddComponent(StrongActorComponentPtr pComponent) {
    std::pair<ActorComponents::iterator, bool> success = m_components.insert(std::make_pair(pComponent->VGetId(), pComponent));
}

std::string Actor::ToXML() {
    return "";
}