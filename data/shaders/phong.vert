#version 450

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    mat4 inv_trans_mv;
    mat4 shadow;
} ubo;

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec3 in_color;
layout(location = 2) in vec3 in_normal;
layout(location = 3) in vec3 in_tangent;
layout(location = 4) in vec2 in_uv;

layout(location = 0) out vec4 pos_vs;
layout(location = 1) out vec3 normal_vs;
layout(location = 2) out vec3 tangent_vs;
layout(location = 3) out vec3 bitangent_vs;
layout(location = 4) out vec2 uv_ts;
layout(location = 5) out vec4 shadow_hs;

void main() {
    pos_vs = ubo.view * ubo.model * vec4(in_pos, 1.0f);
    gl_Position = ubo.proj * pos_vs;
    normal_vs = mat3(ubo.inv_trans_mv) * in_normal;
	tangent_vs = mat3(ubo.inv_trans_mv) * in_tangent;
	bitangent_vs = mat3(ubo.inv_trans_mv) * cross(in_normal, in_tangent);
	uv_ts = in_uv;
	shadow_hs = ubo.shadow * ubo.model * vec4(in_pos, 1.0f);
}