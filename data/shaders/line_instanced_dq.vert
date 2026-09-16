#version 450

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(push_constant) uniform UniformRegisters {
    vec2 u_resolution; // Viewport Size in pixels (e.g. 1920.0, 1080.0)
    float u_line_width; // Dynamic Thickness adjustment in pixels
    vec4 u_color; // Line Color
} registers;

layout(std430, set = 0, binding = 5) readonly buffer JointDualQuats {
    mat2x4 joint_dqs[];
} joint_ssbo;

layout(location = 0) in float in_pc_mix;
layout(location = 1) in float in_side;

layout(location = 3) in uint in_parent_joint_idx;
layout(location = 4) in uint in_child_joint_idx;

const float epsilon = 0.00001f;

vec3 getSkinPos(uint joint) {
    mat2x4 bone = joint_ssbo.joint_dqs[joint];

    vec4 r = bone[0]; // rotation
    vec4 t = bone[1]; // translation

    return vec3(
        2.0 * (-t.w * r.x + t.x * r.w - t.y * r.z + t.z * r.y),
        2.0 * (-t.w * r.y + t.x * r.z + t.y * r.w - t.z * r.x),
        2.0 * (-t.w * r.z - t.x * r.y + t.y * r.x + t.z * r.w)
    );
}

void main() {
    mat4 mvp = ubo.proj * ubo.view * ubo.model;

    vec3 parent_skin_space = getSkinPos(in_parent_joint_idx);
    vec3 child_skin_space = getSkinPos(in_child_joint_idx);

    vec4 position_clip = mvp * vec4(parent_skin_space, 1.0f);
    vec4 target_clip  = mvp * vec4(child_skin_space, 1.0f);

    vec2 position_screen = (position_clip.xy / position_clip.w) * registers.u_resolution;
    vec2 target_screen = (target_clip.xy / target_clip.w) * registers.u_resolution;
    vec4 current_point_clip = mix(position_clip, target_clip, in_pc_mix);
    vec2 current_point_screen = mix(position_screen, target_screen, in_pc_mix);

    vec2 line_direction = target_screen - position_screen;

    if (length(line_direction) > epsilon) {
        line_direction = normalize(line_direction);
    }
    else {
        line_direction = vec2(1.0, 0.0);
    }

    vec2 line_normal = vec2(-line_direction.y, line_direction.x);
    vec2 offset_screen = current_point_screen + line_normal * (registers.u_line_width * 0.5f) * in_side;

    gl_Position = vec4((offset_screen / registers.u_resolution) * current_point_clip.w, current_point_clip.z, current_point_clip.w);
    gl_Position.z = 0.0f;
}