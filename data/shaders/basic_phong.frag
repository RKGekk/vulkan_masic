#version 450

#define MaxLights 9

layout(set = 0, binding = 1) uniform InvMatrixBufferObject {
    mat4 inv_model;
    mat4 inv_view;
    mat4 inv_proj;
} inv_ubo;

layout(set = 0, binding = 2) uniform MaterialBufferObject {
    vec4 u_fresnelR0_roughness;
} material;

layout(set = 0, binding = 3) uniform sampler2D texure_sampler;

layout(push_constant) uniform UniformRegisters {
    vec4 u_ambient_light;
    vec2 u_resolution; // Viewport Size in pixels (e.g. 1920.0, 1080.0)
    uint u_num_dir_lights;
    uint u_num_point_lights;
    uint u_num_spot_lights;
} registers;

struct Light {
    vec4 strength;
    vec4 direction;         // directional/spot light only
    vec4 position;          // point light only
    float falloff_start;    // point/spot light only
    float falloff_end;      // point/spot light only
    float outer_angle;      // spot light only
    float inner_angle;      // spot light only
}; // 56

layout(set = 0, binding = 4) uniform LightBufferObject {
    Light u_light_array[MaxLights];
} light_ubo; // 56 * 9 = 504

layout(location = 0) in vec3 in_normal;
layout(location = 1) in vec4 in_world_pos;
layout(location = 2) in vec2 in_uv;

layout(location = 0) out vec4 out_color;

float CalcAttenuation(float d, float falloff_start, float falloff_end) {
    return clamp((falloff_end - d) / (falloff_end - falloff_start), 0.0f, 1.0f);
}

// Schlick gives an approximation to Fresnel reflectance (see pg. 233 "Real-Time Rendering 3rd Ed.").
// R0 = ( (n-1)/(n+1) )^2, where n is the index of refraction.
vec3 SchlickFresnel(vec3 R0, vec3 normal, vec3 to_light) {
    float cos_cncident_angle = clamp(dot(normal, to_light), 0.0f, 1.0f);

    float f0 = 1.0f - cos_cncident_angle;
    vec3 reflect_percent = R0 + (1.0f - R0) * (f0 * f0 * f0 * f0 * f0);

    return reflect_percent;
}

vec3 BlinnPhong(vec3 light_strength, vec3 to_light, vec3 normal, vec3 to_eye, vec4 diffuse_albedo) {
    const float shininess = 1.0f - material.u_fresnelR0_roughness.a;
    const float m = shininess * 256.0f;
    vec3 half_vec = normalize(to_eye + to_light);

    float roughness_factor = (m + 8.0f) * pow(max(dot(half_vec, normal), 0.0f), m) / 8.0f;
    vec3 fresnel_factor = SchlickFresnel(material.u_fresnelR0_roughness.rgb, half_vec, to_light);

    vec3 spec_albedo = fresnel_factor * roughness_factor;

    // Our spec formula goes outside [0,1] range, but we are 
    // doing LDR rendering.  So scale it down a bit.
    spec_albedo = spec_albedo / (spec_albedo + 1.0f);

    return (diffuse_albedo.rgb + spec_albedo) * light_strength;
}

vec3 ComputeDirectionalLight(Light light_source, vec3 normal, vec3 to_eye, vec4 diffuse_albedo) {
    vec3 to_light = -light_source.direction.xyz;

    // Scale light down by Lambert's cosine law.
    float ndotl = max(dot(to_light, normal), 0.0f);
    vec3 light_strength = light_source.strength.rgb * ndotl;

    return BlinnPhong(light_strength, to_light, normal, to_eye, diffuse_albedo);
}

vec3 ComputePointLight(Light light_source, vec3 point_pos, vec3 normal, vec3 to_eye, vec4 diffuse_albedo) {
    // The vector from the surface to the light.
    vec3 to_light = light_source.position.xyz - point_pos;

    // The distance from surface to light.
    float d = length(to_light);

    // Range test.
    if(d > light_source.falloff_end)
        return vec3(0.0f, 0.0f, 0.0f);

    // Normalize the light vector.
    to_light /= d;

    // Scale light down by Lambert's cosine law.
    float ndotl = max(dot(to_light, normal), 0.0f);
    vec3 light_strength = light_source.strength.rgb * ndotl;

    // Attenuate light by distance.
    float att = CalcAttenuation(d, light_source.falloff_start, light_source.falloff_end);
    light_strength *= att;

    return BlinnPhong(light_strength, to_light, normal, to_eye, diffuse_albedo);
}

vec3 ComputeSpotLight(Light light_source, vec3 point_pos, vec3 normal, vec3 to_eye, vec4 diffuse_albedo) {
    // The vector from the surface to the light.
    vec3 to_light = light_source.position.xyz - point_pos;

    // The distance from surface to light.
    float d = length(to_light);

    // Range test.
    if(d > light_source.falloff_end)
        return vec3(0.0f, 0.0f, 0.0f);

    // Normalize the light vector.
    to_light /= d;

    // Scale light down by Lambert's cosine law.
    float ndotl = max(dot(to_light, normal), 0.0f);
    vec3 light_strength = light_source.strength.rgb * ndotl;

    // Attenuate light by distance.
    float att = CalcAttenuation(d, light_source.falloff_start, light_source.falloff_end);
    light_strength *= att;

    // Scale by spotlight
    float theta = max(dot(-to_light, light_source.direction.xyz), 0.0f);
    float epsilon = cos(light_source.inner_angle - light_source.outer_angle);
    float intensity = clamp((theta - cos(light_source.outer_angle)) / epsilon, 0.0f, 1.0f);

    return BlinnPhong(light_strength, to_light, normal, to_eye, diffuse_albedo);
}

vec4 ComputeLighting(vec3 pos, vec3 normal, vec3 to_eye, vec4 diffuse_albedo) {
    vec3 result = vec3(0.0f, 0.0f, 0.0f);
    uint i = 0;

    for(i = 0; i < registers.u_num_dir_lights; ++i) {
        result += ComputeDirectionalLight(light_ubo.u_light_array[i], normal, to_eye, diffuse_albedo);
    }

    for(i = registers.u_num_dir_lights; i < registers.u_num_dir_lights + registers.u_num_point_lights; ++i) {
        result += ComputePointLight(light_ubo.u_light_array[i], pos, normal, to_eye, diffuse_albedo);
    }

    for(i = registers.u_num_dir_lights + registers.u_num_point_lights; i < registers.u_num_dir_lights + registers.u_num_point_lights + registers.u_num_spot_lights; ++i) {
        result += ComputeSpotLight(light_ubo.u_light_array[i], pos, normal, to_eye, diffuse_albedo);
    }

    return vec4(result, 0.0f);
}

void main() {
    vec4 diffuse_albedo = texture(texure_sampler, in_uv);

    vec3 world_normal = normalize(in_normal);
    vec3 world_eye_position = inv_ubo.inv_view[3].xyz;
    vec3 world_to_eye = normalize(world_eye_position - in_world_pos.xyz);

    vec4 diffuse_attenuation = ComputeLighting(in_world_pos.xyz, in_normal, world_to_eye, diffuse_albedo);

    vec4 ambient = registers.u_ambient_light * diffuse_albedo;
    vec4 lit_color = ambient + diffuse_attenuation;

    // Common convention to take alpha from diffuse material.
    lit_color.a = diffuse_albedo.a;

    //out_color = texture(texure_sampler, in_uv);
    out_color = lit_color;
}