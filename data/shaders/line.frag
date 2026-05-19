#version 450

layout (location = 0) in vec4 in_color;
layout (location = 0) out vec4 out_color;
layout (constant_id = 0) const bool kNonLinearColorSpace = true;

void main() {
    vec4 c = vec4(in_color.rgb, 1.0);
    // Render UI in linear color space to sRGB framebuffer.
    out_color = kNonLinearColorSpace ? vec4(pow(c.rgb, vec3(2.2)), c.a) : c;
}