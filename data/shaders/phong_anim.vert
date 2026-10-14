#version 450

layout(set = 0, binding = 0) uniform CameraBufferObject {
    mat4 view;
    mat4 proj;
    mat4 inv_view;
    mat4 inv_proj;
} camera_ubo;

layout(set = 0, binding = 1) uniform ModelBufferObject {
    mat4 model;
    mat4 inv_model;
} model_ubo;

layout(std430, set = 0, binding = 5) readonly buffer JointBufferObject {
    mat4 joint_array[];
} joint_ubo;

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec3 in_tangent;
layout(location = 3) in vec2 in_uv;
layout(location = 4) in uvec4 in_joint_indices;
layout(location = 5) in vec4 in_joint_weights;

layout(location = 0) out vec3 out_normal;
layout(location = 1) out vec3 out_tangent;
layout(location = 2) out vec4 out_world_pos;
layout(location = 3) out vec2 out_uv;

void main() {
    mat4 skin = in_joint_weights.x * joint_ubo.joint_array[in_joint_indices.x]
              + in_joint_weights.y * joint_ubo.joint_array[in_joint_indices.y]
              + in_joint_weights.z * joint_ubo.joint_array[in_joint_indices.z]
              + in_joint_weights.w * joint_ubo.joint_array[in_joint_indices.w];

    mat3 skin_only_rot = mat3(skin);

    gl_Position = camera_ubo.proj * camera_ubo.view * model_ubo.model * skin * vec4(in_position, 1.0f);
    
    out_normal = transpose(mat3(model_ubo.inv_model)) * skin_only_rot * in_normal;
    out_tangent = transpose(mat3(model_ubo.inv_model)) * skin_only_rot * in_tangent;
    out_world_pos = model_ubo.model * skin * vec4(in_position, 1.0f);
    out_uv = in_uv;
}