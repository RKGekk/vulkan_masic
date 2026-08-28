#include "inverse_kinematics_component.h"

#include "../../tools/string_tools.h"
#include "../../application.h"
#include "model_component.h"

const std::string InverseKinematicsComponent::g_name = "InverseKinematicsComponent";

InverseKinematicsComponent::InverseKinematicsComponent() {
    using namespace std::literals;

    const std::shared_ptr<Scene>& scene_ptr = Application::Get().GetGameLogic()->GetHumanView()->VGetScene();
    m_iksolver = scene_ptr->getIKSolver();
}

InverseKinematicsComponent::InverseKinematicsComponent(const pugi::xml_node& data) {
    using namespace std::literals;

    const std::shared_ptr<Scene>& scene_ptr = Application::Get().GetGameLogic()->GetHumanView()->VGetScene();
    m_iksolver = scene_ptr->getIKSolver();

    Init(data);
}

InverseKinematicsComponent::~InverseKinematicsComponent() {
    
}

const std::string& InverseKinematicsComponent::VGetName() const {
    return g_name;
}

const ComponentDependecyList& InverseKinematicsComponent::VGetComponentDependecy() const {
    static const ComponentDependecyList component_dep = {ModelComponent::g_name};
    return component_dep;
}

pugi::xml_node InverseKinematicsComponent::VGenerateXml() {
    return pugi::xml_node();
}

void InverseKinematicsComponent::VPostInit() {
    
}

void InverseKinematicsComponent::Apply() {
    for(const auto&[targen_name, target_system] : m_iksolver->getTargetMap()) {
        m_iksolver->recalcTarget(targen_name);
        m_iksolver->solveCCD(targen_name);
        m_iksolver->applySolution(targen_name);
    }
}

void InverseKinematicsComponent::SetTarget(glm::vec3 target) {
    for(const auto&[targen_name, target_system] : m_iksolver->getTargetMap()) {
        m_iksolver->setTarget(targen_name, target);
        m_iksolver->recalcTarget(targen_name);
        m_iksolver->solveCCD(targen_name);
        m_iksolver->applySolution(targen_name);
    }
}

glm::vec3 InverseKinematicsComponent::getTarget() const {
    return (*m_iksolver->getTargetMap().begin()).second->world_target;
}

bool InverseKinematicsComponent::Init(const pugi::xml_node& data) {
    //const std::shared_ptr<Scene>& scene_ptr = Application::Get().GetGameLogic()->GetHumanView()->VGetScene();
    std::shared_ptr<Actor> act = GetOwner();
	std::shared_ptr<ModelComponent> mc = act->GetComponent<ModelComponent>(ActorComponent::GetIdFromName("ModelComponent")).lock();
	if (!mc) return false;

    const std::shared_ptr<SceneNode>& root_model_node = mc->VGetSceneNode();

    for (pugi::xml_node target_system_node = data.first_child(); target_system_node; target_system_node = target_system_node.next_sibling()) {

        std::shared_ptr<InverseKinematicsSolver::TargetSystem> target_system = std::make_shared<InverseKinematicsSolver::TargetSystem>();
        std::string ts_name = target_system_node.attribute("name").as_string();
        target_system->name = ts_name;
        target_system->world_target = posfromattr3f(target_system_node.child("world_target"));
        target_system->iterations = target_system_node.child("iterations").text().as_uint();
        target_system->tolerance = target_system_node.child("tolerance").text().as_float();

        std::string root_tp_name = target_system_node.child("root_turning_point_node_name").text().as_string();
        const std::shared_ptr<SceneNode>& root_tp_node = root_model_node->FindIf([&root_tp_name](const std::shared_ptr<SceneNode>& node){ return node->Get().Name() == root_tp_name; });
        target_system->root_turning_point = root_tp_node;

        std::string effector_node_name = target_system_node.child("effector_node_name").text().as_string();
        const std::shared_ptr<SceneNode>& effector_node = root_model_node->FindIf([&effector_node_name](const std::shared_ptr<SceneNode>& node){ return node->Get().Name() == effector_node_name; });
        target_system->effector_node = effector_node;

        m_iksolver->addTarget(std::move(target_system));
        //m_iksolver->solveCCD(ts_name);
    }

	m_initialized = true;

    return m_initialized;
}

bool InverseKinematicsComponent::VInit(const pugi::xml_node& pData) {
    return Init(pData);
}