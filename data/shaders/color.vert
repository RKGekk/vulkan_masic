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

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 fragColor;

void main() {
    gl_Position = camera_ubo.proj * camera_ubo.view * model_ubo.model * vec4(inPosition, 1.0f);
    gl_Position.z = 0.0f;
    fragColor = inColor;
}