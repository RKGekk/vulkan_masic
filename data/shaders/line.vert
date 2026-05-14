#version 450

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo_mvp;

layout(set = 0, binding = 1) uniform UniformBufferObject {
    vec2 u_resolution; // Viewport Size in pixels (e.g. 1920.0, 1080.0)
    float u_lineWidth; // Dynamic Thickness adjustment in pixels
} ubo_param;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec4 inColor;
layout(location = 2) in vec3 inTarget;
layout(location = 3) in float inSide;

layout(location = 0) out vec3 fragColor;

void main() {
    gl_Position = ubo_mvp.proj * ubo_mvp.view * ubo_mvp.model * vec4(inPosition, 1.0f);
    gl_Position.z = 0.0f;
    fragColor = inColor;
}