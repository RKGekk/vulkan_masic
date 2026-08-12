#version 460 core

#define MaxBones 96

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

layout(std430, set = 0, binding = 5) readonly buffer JointDualQuats {
    mat2x4 joint_dqs[];
} joint_ssbo;

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

mat2x4 getJointTransform(uvec4 joints, vec4 weights) {
    // read dual quaterions from buffer
    mat2x4 dq0 = joint_ssbo.joint_dqs[joints.x];
    mat2x4 dq1 = joint_ssbo.joint_dqs[joints.y];
    mat2x4 dq2 = joint_ssbo.joint_dqs[joints.z];
    mat2x4 dq3 = joint_ssbo.joint_dqs[joints.w];

    // shortest rotation
    weights.y *= sign(dot(dq0[0], dq1[0]));
    weights.z *= sign(dot(dq0[0], dq2[0]));
    weights.w *= sign(dot(dq0[0], dq3[0]));

    // blend
    mat2x4 result = weights.x * dq0
                  + weights.y * dq1
                  + weights.z * dq2
                  + weights.w * dq3;

    // normalize the dual quaternion
    float norm = length(result[0]);
    return result / norm;
}

mat4 getSkinMatrix() {
    mat2x4 bone = getJointTransform(in_joint_indices, in_joint_weights);

    vec4 r = bone[0]; // rotation
    vec4 t = bone[1]; // translation

    return mat4(
        1.0 - (2.0 * r.y * r.y) - (2.0 * r.z * r.z),                  (2.0 * r.x * r.y) + (2.0 * r.w * r.z),                  (2.0 * r.x * r.z) - (2.0 * r.w * r.y),            0.0,
              (2.0 * r.x * r.y) - (2.0 * r.w * r.z),            1.0 - (2.0 * r.x * r.x) - (2.0 * r.z * r.z),                  (2.0 * r.y * r.z) + (2.0 * r.w * r.x),            0.0,
              (2.0 * r.x * r.z) + (2.0 * r.w * r.y),                  (2.0 * r.y * r.z) - (2.0 * r.w * r.x),            1.0 - (2.0 * r.x * r.x) - (2.0 * r.y * r.y),            0.0,
        2.0 * (-t.w * r.x + t.x * r.w - t.y * r.z + t.z * r.y), 2.0 * (-t.w * r.y + t.x * r.z + t.y * r.w - t.z * r.x), 2.0 * (-t.w * r.z - t.x * r.y + t.y * r.x + t.z * r.w), 1.0
    );
}

void main() {
    mat4 skin = getSkinMatrix();

    mat3 skin_only_rot = mat3(skin);

    gl_Position = ubo.proj * ubo.view * ubo.model * skin * vec4(in_position, 1.0f);
    
    out_normal = transpose(mat3(inv_ubo.inv_model)) * skin_only_rot * in_normal;
    out_tangent = transpose(mat3(inv_ubo.inv_model)) * skin_only_rot * in_tangent;
    out_world_pos = ubo.model * skin * vec4(in_position, 1.0f);
    out_uv = in_uv;
}