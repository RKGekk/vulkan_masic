#pragma once

#include "../../tools/glm_define.h"

#include <glm/glm.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext.hpp>

struct LightNodeProperties {
    glm::vec4 strength;
    glm::vec4 direction;    // directional/spot light only
    glm::vec4 position;     // point light only
    float falloff_start;    // point/spot light only
    float falloff_end;      // point/spot light only
    float outer_angle;      // spot light only
    float inner_angle;      // spot light only
};