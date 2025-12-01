#include "actor_factory.h"

#include "transform_component.h"
#include "camera_component.h"
#include "model_component.h"

unsigned int ActorFactory::GetNextActorId() {
    return ++m_last_actorId;
}

ActorFactory::ActorFactory() {
    m_last_actorId = 0;

    m_component_factory.Register<TransformComponent>(ActorComponent::GetIdFromName(TransformComponent::g_name), TransformComponent::g_name);
    m_component_factory.Register<CameraComponent>(ActorComponent::GetIdFromName(CameraComponent::g_name), CameraComponent::g_name);
    m_component_factory.Register<ModelComponent>(ActorComponent::GetIdFromName(ModelComponent::g_name), ModelComponent::g_name);
}

std::unordered_map<std::string, std::pair<std::shared_ptr<ActorComponent>, pugi::xml_node>> ActorFactory::getAllComponents(pugi::xml_node actor_node) {
    std::unordered_map<std::string, std::pair<std::shared_ptr<ActorComponent>, pugi::xml_node>> result;

    for (pugi::xml_node node = actor_node.first_child(); node; node = node.next_sibling()) {
        const char* name_cstr = node.name();
        std::string name(name_cstr);
        std::shared_ptr<ActorComponent> pComponent(m_component_factory.Create(ActorComponent::GetIdFromName(name)));
        result[name] = std::make_pair(pComponent, node);
    }

    return result;
}

void initializeComponent(const std::string& name, std::shared_ptr<Actor> pActor, const std::unordered_map<std::string, std::pair<std::shared_ptr<ActorComponent>, pugi::xml_node>>& all_components) {
    
    auto&[pComponent, xml_data] = all_components.at(name);
    if (!pComponent || pComponent->GetIsInitialized()) return;
    
    const ComponentDependecyList& dep = pComponent->VGetComponentDependecy();
    for(const auto& dep_cp_name : dep) {
        std::shared_ptr<ActorComponent> pDepComponent = all_components.at(dep_cp_name).first;
        if (!pDepComponent || pDepComponent->GetIsInitialized()) continue;
        
        initializeComponent(dep_cp_name, pActor, all_components);
    }

    pComponent->SetOwner(pActor);
    pComponent->VInit(xml_data);
    pActor->AddComponent(pComponent);
}

std::shared_ptr<Actor> ActorFactory::CreateActor(const pugi::xml_node& actor_data, const ActorId servers_actorId) {

    ActorId next_actorId = servers_actorId;
    if (next_actorId == 0) {
        next_actorId = GetNextActorId();
    }

    std::shared_ptr<Actor> pActor = std::make_shared<Actor>(next_actorId);
    if (!pActor->Init(actor_data)) {
        return std::shared_ptr<Actor>();
    }

    std::unordered_map<std::string, std::pair<std::shared_ptr<ActorComponent>, pugi::xml_node>> all_components = getAllComponents(actor_data);
    for (auto[comp_name, data] : all_components) {
        initializeComponent(comp_name, pActor, all_components);
    }

    pActor->PostInit();

    return pActor;
}

std::shared_ptr<ActorComponent> ActorFactory::VCreateComponent(const pugi::xml_node& data, std::unordered_map<std::string, std::pair<std::shared_ptr<ActorComponent>, pugi::xml_node>> all_components) {
    const char* name = data.name();
    std::shared_ptr<ActorComponent> pComponent(m_component_factory.Create(ActorComponent::GetIdFromName(name)));

    if (pComponent) {
        if (!pComponent->VInit(data)) {
            return std::shared_ptr<ActorComponent>();
        }
    }
    else {
        return std::shared_ptr<ActorComponent>();
    }

    return pComponent;
}