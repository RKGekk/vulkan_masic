#include "inverse_kinematics_solver.h"

#include "nodes/scene_node.h"

InverseKinematicsSolver::InverseKinematicsSolver() {}

bool InverseKinematicsSolver::solveCCD(const TargetName& target_name) {
    if(!m_solutions_map.contains(target_name)) return false;

    const std::shared_ptr<TargetSystem>& ts = m_target_map.at(target_name);
    const std::shared_ptr<SceneNode>& effector_node = ts->effector_node;

    glm::vec3 effector_world_pos = effector_node->Get().ToRootTranslation3();
    glm::vec3 target_world_pos = ts->world_target;
    float treshold = ts->treshold;
    float distance = glm::length(target_world_pos - effector_world_pos);
    if(distance < treshold) return true;

    const std::unordered_map<std::shared_ptr<SceneNode>, glm::mat4>& solution = m_solutions_map.at(target_name);
    size_t iterations = ts->iterations;
    std::shared_ptr<SceneNode>& current_node = ts->effector_node;
    for(size_t i = 0u; i < iterations; ++i) {

        glm::vec3 current_world_pos = current_node->Get().ToRootTranslation3();
        glm::quat current_rotation = current_node->Get().ToRootRotation();

        glm::vec3 to_effector = glm::normalize(effector_world_pos - current_world_pos);
        glm::vec3 to_target = glm::normalize(target_world_pos - current_world_pos);
        glm::quat effector_to_target = glm::rotation(to_effector, to_target);
        glm::quat local_rotation_op = current_rotation * effector_to_target * glm::conjugate(current_rotation);
        glm::quat local_rotation = current_node->Get().ToParentRotation() * local_rotation_op;

        solution[current_node] = glm::mat4_cast(local_rotation);
    }

    return true;
}

void InverseKinematicsSolver::addTarget(std::shared_ptr<TargetSystem> ts) {
    const std::shared_ptr<Scene> scene = ts->root_turning_point->GetScene();
    std::shared_ptr<SceneNode>& current_node = ts->effector_node;
    while(current_node != ts->root_turning_point) {

        m_solutions_map[ts->name][current_node] = current_node->Get().ToParent();
        //m_solutions_map[ts->name][current_node] = glm::mat4(1.0f);
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