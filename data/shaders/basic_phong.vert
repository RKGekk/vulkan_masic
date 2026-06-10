#version 450

layout(set = 0, binding = 0) uniform MatrixBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(set = 0, binding = 1) uniform InvMatrixBufferObject {
    mat4 inv_model;
    mat4 inv_view;
    mat4 inv_proj;
} inv_ubo;

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec3 in_tangent;
layout(location = 3) in vec2 in_uv;

layout(location = 0) out vec3 out_normal;
layout(location = 1) out vec4 out_world_pos;
layout(location = 2) out vec2 out_uv;

void main() {
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(in_position, 1.0f);
    
    out_normal = transpose(mat3(inv_ubo.inv_model)) * in_normal;
    out_world_pos = ubo.model * vec4(in_position, 1.0f);
    out_uv = in_uv;
}