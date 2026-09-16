#version 450

layout (location = 0) out vec4 out_color;
layout (constant_id = 0) const bool kNonLinearColorSpace = true;

layout(push_constant) uniform UniformRegisters {
    vec2 u_resolution; // Viewport Size in pixels (e.g. 1920.0, 1080.0)
    float u_line_width; // Dynamic Thickness adjustment in pixels
    vec4 u_color; // Line Color
} registers;


void main() {
    vec4 c = vec4(registers.u_color.rgb, 1.0);
    // Render UI in linear color space to sRGB framebuffer.
    out_color = kNonLinearColorSpace ? vec4(pow(c.rgb, vec3(2.2)), c.a) : c;
}