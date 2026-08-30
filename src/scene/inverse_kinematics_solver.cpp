#include "inverse_kinematics_solver.h"

#include "nodes/scene_node.h"
#include "../tools/math_tools.h"

InverseKinematicsSolver::InverseKinematicsSolver() {}

struct StackFrame {
    std::shared_ptr<SceneNode> node;

};

bool InverseKinematicsSolver::solveCCD(const TargetName& target_name) {
    if(!m_target_map.contains(target_name)) return false;
    if(!m_solutions_map.contains(target_name)) return false;
    std::vector<SolutionPart>& solution_part = m_solutions_map[target_name];
    if(solution_part.size() == 0u) return false;

    const std::shared_ptr<TargetSystem>& ts = m_target_map[target_name];

    SolutionPart& effector_sol = solution_part[0u];
    glm::vec3 effector_world_pos = glm::vec3(effector_sol.gloabal_transform[3u]);
    glm::vec3 target_world_pos = ts->world_target;
    float tolerance = ts->tolerance;
    //float distance = glm::length(target_world_pos - effector_world_pos);
    //if(distance < tolerance) return true;
    //if(solution_part.size() == 1u) return false;

    size_t iterations = ts->iterations;
    size_t sz = solution_part.size();
    for(size_t iteration = 0u; iteration < iterations; ++iteration) {
        for(size_t i = 1u; i < sz; ++i) {
            
            float distance = glm::length(target_world_pos - effector_world_pos);
            if(distance < tolerance) return true;

            SolutionPart& s = solution_part[i];
            
            //effector_world_pos = glm::vec3(effector_sol.gloabal_transform[3u]);
            glm::vec3 current_world_pos = glm::vec3(s.gloabal_transform[3u]);
            //glm::quat current_world_rotation = glm::quat_cast(s.gloabal_transform);
            glm::quat current_world_rotation = getRotation(s.gloabal_transform);

            glm::vec3 to_effector = glm::normalize(effector_world_pos - current_world_pos);
            glm::vec3 to_target = glm::normalize(target_world_pos - current_world_pos);
            glm::quat effector_to_target = glm::rotation(to_effector, to_target);

            glm::quat co_rotation = glm::conjugate(current_world_rotation);
            //glm::quat local_rotation_op = current_world_rotation * effector_to_target * co_rotation;
            glm::quat local_rotation_op = co_rotation * effector_to_target * current_world_rotation;
            glm::mat4 local_rot_mat = glm::mat4_cast(local_rotation_op);
            s.local_transform = s.local_transform * local_rot_mat;
            //s.local_transform = glm::mat4_cast(local_rotation_op) * s.local_transform;
            //s.local_transform = s.local_transform * glm::mat4_cast(effector_to_target);

            for(int j = i; j > 0; --j) {
                SolutionPart& sj = solution_part[j];
                sj.gloabal_transform = sj.parent_gloabal_transform * sj.local_transform;
                SolutionPart& sjc = solution_part[j-1];
                sjc.parent_gloabal_transform = sj.gloabal_transform;
            }
            effector_sol.gloabal_transform = effector_sol.parent_gloabal_transform * effector_sol.local_transform;

            effector_world_pos = glm::vec3(effector_sol.gloabal_transform[3u]);
        }
    }

    return true;
}

void InverseKinematicsSolver::addTarget(std::shared_ptr<TargetSystem> ts) {
    std::string ts_name = ts->name;
    m_target_map[ts_name] = std::move(ts);
    recalcTarget(ts_name);
}

void InverseKinematicsSolver::recalcTarget(const TargetName& target_name) {
    if(!m_target_map.contains(target_name)) return;
    const std::shared_ptr<TargetSystem>& ts = m_target_map[target_name];
    const std::shared_ptr<Scene> scene = ts->root_turning_point->GetScene();
    std::shared_ptr<SceneNode>& current_node = ts->effector_node;
    if(!m_solutions_map.contains(target_name)) {
        std::vector<SolutionPart>& solutions = m_solutions_map[target_name];
        while(current_node != ts->root_turning_point) {
            solutions.push_back({current_node, current_node->Get().ToParent(), current_node->Get().ToRoot(), current_node->GetParent()->Get().ToRoot()});
            current_node = current_node->GetParent();
        }
        solutions.push_back({ts->root_turning_point, ts->root_turning_point->Get().ToParent(), ts->root_turning_point->Get().ToRoot(), ts->root_turning_point->GetParent()->Get().ToRoot()});
    }
    else {
        std::vector<SolutionPart>& solutions = m_solutions_map[target_name];
        size_t sz = solutions.size();
        for(size_t i = 0u; i < sz; ++i) {
            SolutionPart& s = solutions[i];
            //s.node = current_node;
            s.local_transform = s.node->Get().ToParent();
            s.gloabal_transform = s.node->Get().ToRoot();
            s.parent_gloabal_transform = s.node->GetParent()->Get().ToRoot();
            //current_node = current_node->GetParent();
        }
    }
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

const std::unordered_map<InverseKinematicsSolver::TargetName, std::shared_ptr<InverseKinematicsSolver::TargetSystem>>& InverseKinematicsSolver::getTargetMap() const {
    return m_target_map;
}

void InverseKinematicsSolver::setTarget(const TargetName& target_name, glm::vec3 world_target) {
    if(!m_target_map.contains(target_name)) return;
    m_target_map[target_name]->world_target = world_target;
    solveCCD(target_name);
    applySolution(target_name);
}