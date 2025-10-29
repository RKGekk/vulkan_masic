#version 450

layout (location = 0) in vec4 in_color;
layout (location = 1) in vec2 in_uv;

layout (location = 0) out vec4 out_color;

const bool kNonLinearColorSpace = false;

layout(set = 0, binding = 0) uniform UniformBufferObject {
    vec4 LRTB;
} ubo;

layout(set = 0, binding = 1) uniform sampler2D texSampler;

void main() {
    vec4 c = in_color * texture(texSampler, in_uv);
    // Render UI in linear color space to sRGB framebuffer.
    out_color = kNonLinearColorSpace ? vec4(pow(c.rgb, vec3(2.2)), c.a) : c;
}