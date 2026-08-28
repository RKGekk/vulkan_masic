#pragma once

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

#include "scene.h"

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

class InverseKinematicsSolver {
public:
    using TargetName = std::string;

    struct TargetSystem {
        TargetName name;
        glm::vec3 world_target;
        size_t iterations;
        float tolerance;
        std::shared_ptr<SceneNode> root_turning_point;
        std::shared_ptr<SceneNode> effector_node;
    };

    struct SolutionPart {
        std::shared_ptr<SceneNode> node;
        glm::mat4 local_transform;
        glm::mat4 gloabal_transform;
        glm::mat4 parent_gloabal_transform;
    };

    InverseKinematicsSolver();

    bool solveCCD(const TargetName& target_name);

    void addTarget(std::shared_ptr<TargetSystem> ts);
    void recalcTarget(const TargetName& target_name);
    void applySolution(const TargetName& target_name);
    const std::vector<SolutionPart>& getSolution(const TargetName& target_name) const;

private:
    std::unordered_map<TargetName, std::shared_ptr<TargetSystem>> m_target_map;
    std::unordered_map<TargetName, std::vector<SolutionPart>> m_solutions_map;
};