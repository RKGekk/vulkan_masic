#version 450

struct Vertex {
    float x;
    float y;
    float u;
    float v;
    uint rgba;
};

layout (location = 0) out vec4 out_color;
layout (location = 1) out vec2 out_uv;

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inTexCoords;
layout(location = 2) in uint inColor;

layout(set = 0, binding = 0) uniform UniformBufferObject {
    vec4 LRTB;
} ubo;

void main() {
    float L = ubo.LRTB.x;
    float R = ubo.LRTB.y;
    float T = ubo.LRTB.z;
    float B = ubo.LRTB.w;
    mat4 proj = mat4(
        2.0 / (R - L),     0.0,               0.0, 0.0,
        0.0,               2.0 / (T - B),     0.0, 0.0,
        0.0,               0.0,              -1.0, 0.0,
        (R + L) / (L - R), (T + B) / (B - T), 0.0, 1.0
    );
    Vertex v = Vertex(inPosition.x, inPosition.y, inTexCoords.x, inTexCoords.y, inColor);
    out_color = unpackUnorm4x8(v.rgba);
    out_uv = vec2(v.u, v.v);
    gl_Position = proj * vec4(v.x, v.y, 0, 1);
}