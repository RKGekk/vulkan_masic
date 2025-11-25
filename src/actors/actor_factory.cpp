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

void initializeComponent(const std::string& name, const std::unordered_map<std::string, std::pair<std::shared_ptr<ActorComponent>, pugi::xml_node>>& all_components) {
    for (auto[comp_name, data] : all_components) {
        std::shared_ptr<ActorComponent> pComponent = data.first;
        if (!pComponent || pComponent->GetIsInitialized()) continue;
        
        const ComponentDependecyList& dep = pComponent->VGetComponentDependecy();
        for(const auto& dep_cp_name : dep) {
            initializeComponent(dep_cp_name, all_components);
        }

        pugi::xml_node xml_data = data.second;
        pComponent->VInit(xml_data);
    }
}

std::shared_ptr<Actor> ActorFactory::CreateActor(const std::string& actor_resource, const pugi::xml_node& overrides, const ActorId servers_actorId) {

    pugi::xml_document xml_doc;
    pugi::xml_parse_result parse_res = xml_doc.load_file(actor_resource.c_str());
    if (!parse_res) return std::shared_ptr<Actor>();

    pugi::xml_node root_node = xml_doc.root();
    if (!root_node) return std::shared_ptr<Actor>();

    pugi::xml_node actor_node = root_node.child("Actor");
    if (!actor_node) return std::shared_ptr<Actor>();

    ActorId next_actorId = servers_actorId;
    if (next_actorId == 0) {
        next_actorId = GetNextActorId();
    }
    std::shared_ptr<Actor> pActor(new Actor(next_actorId));
    if (!pActor->Init(actor_node)) {
        return std::shared_ptr<Actor>();
    }

    bool initial_transform_set = false;

    std::unordered_map<std::string, std::pair<std::shared_ptr<ActorComponent>, pugi::xml_node>> all_components = getAllComponents(actor_node);
    for (auto[comp_name, data] : all_components) {
        std::shared_ptr<ActorComponent> pComponent = data.first;
        initializeComponent(comp_name, all_components);
        if (pComponent) {
            pActor->AddComponent(pComponent);
            pComponent->SetOwner(pActor);
        }
        else {
            return std::shared_ptr<Actor>();
        }
    }

    if (overrides) {
        ModifyActor(pActor, overrides);
    }

    pActor->PostInit();

    return pActor;
}

std::shared_ptr<ActorComponent> ActorFactory::VCreateComponent(const pugi::xml_node& data) {
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

void ActorFactory::ModifyActor(std::shared_ptr<Actor> pActor, const pugi::xml_node& overrides) {
    if (overrides.attribute("name")) {
        pActor->SetName(overrides.attribute("name").as_string());
    };
    for (pugi::xml_node node = overrides.first_child(); node; node = node.next_sibling()) {
        unsigned int componentId = ActorComponent::GetIdFromName(node.name());
        std::shared_ptr<ActorComponent> pComponent = MakeStrongPtr(pActor->GetComponent<ActorComponent>(componentId));
        if (pComponent) {
            pComponent->VInit(node);
            pComponent->VOnChanged();
        }
        else {
            pComponent = VCreateComponent(node);
            if (pComponent) {
                pActor->AddComponent(pComponent);
                pComponent->SetOwner(pActor);
            }
        }
    }
}