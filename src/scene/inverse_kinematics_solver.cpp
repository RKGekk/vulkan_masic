#include "inverse_kinematics_solver.h"

#include "nodes/scene_node.h"

InverseKinematicsSolver::InverseKinematicsSolver() {}

struct StackFrame {
    std::shared_ptr<SceneNode> node;

};

bool InverseKinematicsSolver::solveCCD(const TargetName& target_name) {
    if(!m_solutions_map.contains(target_name)) return false;
    std::vector<SolutionPart>& solution_part = m_solutions_map[target_name];
    if(solution_part.size() == 0u) return false;

    const std::shared_ptr<TargetSystem>& ts = m_target_map.at(target_name);
    const std::shared_ptr<SceneNode>& effector_node = ts->effector_node;
    const std::shared_ptr<SceneNode>& root_turning_point = ts->root_turning_point;

    glm::vec3 effector_world_pos = effector_node->Get().ToRootTranslation3();
    glm::vec3 target_world_pos = ts->world_target;
    float tolerance = ts->tolerance;
    float distance = glm::length(target_world_pos - effector_world_pos);
    if(distance < tolerance) return true;
    if(solution_part.size() == 1u) return false;

    int current_sol_idx = solution_part.size() - 1u;
    size_t iterations = ts->iterations;
    std::shared_ptr<SceneNode>& current_node = ts->effector_node;
    for(size_t i = 0u; i < iterations; ++i) {
        while(current_node != root_turning_point) {
            effector_world_pos = effector_node->Get().ToRootTranslation3();
            glm::vec3 current_world_pos = current_node->Get().ToRootTranslation3();
            glm::quat current_rotation = current_node->Get().ToRootRotation();

            glm::vec3 to_effector = glm::normalize(effector_world_pos - current_world_pos);
            glm::vec3 to_target = glm::normalize(target_world_pos - current_world_pos);
            glm::quat effector_to_target = glm::rotation(to_effector, to_target);
            glm::quat local_rotation_op = current_rotation * effector_to_target * glm::conjugate(current_rotation);
            //glm::quat local_rotation = current_node->Get().ToParentRotation() * local_rotation_op;

            //solution_part[current_sol_idx].local_transform = glm::mat4_cast(local_rotation);
            for(int j = current_sol_idx; j >= 0; --j) {
                glm::mat4 local_transform = solution_part[current_sol_idx].local_transform * glm::mat4_cast(local_rotation_op);
                solution_part[current_sol_idx].local_transform = local_transform;
            }

            ++current_sol_idx;
            //current_node = current_node->GetParent();
            current_node = solution_part[current_sol_idx].node;
        }
    }

    return true;
}

void InverseKinematicsSolver::addTarget(std::shared_ptr<TargetSystem> ts) {
    const std::shared_ptr<Scene> scene = ts->root_turning_point->GetScene();
    std::shared_ptr<SceneNode>& current_node = ts->effector_node;
    while(current_node != ts->root_turning_point) {

        m_solutions_map[ts->name].push_back({current_node, current_node->Get().ToParent()});
        //m_solutions_map[ts->name].push_back({current_node, glm::mat4(1.0f)});
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