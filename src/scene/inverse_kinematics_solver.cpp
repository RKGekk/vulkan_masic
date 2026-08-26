#include "inverse_kinematics_solver.h"

#include "nodes/scene_node.h"

InverseKinematicsSolver::InverseKinematicsSolver() {}

void InverseKinematicsSolver::solve(const TargetName& target_name) {
    
}

void InverseKinematicsSolver::addTarget(std::shared_ptr<TargetSystem> ts) {
    const std::shared_ptr<Scene> scene = ts->root_turning_point->GetScene();
    std::shared_ptr<SceneNode>& current_node = ts->effector_node;
    while(current_node != ts->root_turning_point) {

        m_solutions_map[ts->name].push_back({current_node, glm::mat4(1.0f), false});
        current_node = current_node->GetParent();
    }

    m_target_map[ts->name] = std::move(ts);
}

void InverseKinematicsSolver::applySolution(const TargetName& target_name) {
    if(!m_solutions_map.contains(target_name)) return;

    for(const SolutionPart& solution : m_solutions_map[target_name]) {
        solution.node->SetTransform(solution.local_transform);
    }
}

const std::vector<InverseKinematicsSolver::SolutionPart>& InverseKinematicsSolver::getSolution(const TargetName& target_name) const {
    return m_solutions_map.at(target_name);
}