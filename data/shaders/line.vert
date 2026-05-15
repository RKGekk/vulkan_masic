#version 450

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(push_constant) uniform UniformBufferObject {
    vec2 u_resolution; // Viewport Size in pixels (e.g. 1920.0, 1080.0)
    float u_line_width; // Dynamic Thickness adjustment in pixels
} registers;

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec4 in_color;
layout(location = 2) in vec3 in_target;
layout(location = 3) in float in_side;

layout(location = 0) out vec3 out_Color;

const float epsilon = 0.00001f;

void main() {
    mat4 mvp = ubo.proj * ubo.view * ubo.model;

    vec4 position_clip = mvp * vec4(in_position, 1.0f);
    vec4 target_clip  = mvp * vec4(in_target, 1.0f);

    vec2 position_screen = (position_clip.xy / position_clip.w) * registers.u_resolution;
    vec2 target_screen = (target_clip.xy / target_clip.w) * registers.u_resolution;

    vec2 line_direction = target_screen - position_screen;

    if (length(line_direction) > epsilon) {
        line_direction = normalize(line_direction);
    }
    else {
        line_direction = vec2(1.0, 0.0);
    }

    vec2 line_normal = vec2(-line_direction.y, line_direction.x);
    vec2 offset_screen = position_screen + line_normal * (registers.u_line_width * 0.5f) * in_side;

    gl_Position = vec4((offset_screen / registers.u_resolution) * position_clip.w, position_clip.z, position_clip.w);
    fragColor = in_color;
}