#version 450

layout(set = 0, binding = 1) uniform MaterialObject {
    vec4 diffuse;
    //------------------------------------ ( 16 bytes )
	vec4 specular;
    //------------------------------------ ( 16 bytes )
	vec4 emissive;
    //------------------------------------ ( 16 bytes )
	vec4 ambient;
    //------------------------------------ ( 16 bytes )
	vec4 reflectance;
    //------------------------------------ ( 16 bytes )
	float opacity;        // If Opacity < 1, then the material is transparent.
	float specular_power;
	float ior;            // For transparent materials, IOR > 0.
	float bump_intensity; // When using bump textures (height maps) we need
                          // to scale the height values so the normals are visible.
    //------------------------------------ ( 16 bytes )
    float metallic_factor;
	float roughness_factor;
    uint padding1;
	uint padding2;
    //------------------------------------ ( 16 bytes )
    uint has_texture;
	uint padding3;
    uint padding4;
    uint padding5;
    // Total:                              ( 16 * 8 = 128 bytes )
} material;

struct PointLight {
    vec4 pos_ws; // Light position in world space.
    //----------------------------------- (16 byte boundary)
    vec4 pos_vs; // Light position in view space.
    //----------------------------------- (16 byte boundary)
    vec4 color;
    //----------------------------------- (16 byte boundary)
    float const_att;
    float lin_att;
    float quad_att;
    float Padding1;
    //----------------------------------- (16 byte boundary)
    vec3 ambient;
    float padding2;
    //----------------------------------- (16 byte boundary)
    // Total:                              16 * 5 = 80 bytes
};

layout(set = 0, binding = 2) uniform PointLightObject {
    PointLight obj;
} point_light;

struct SpotLight {
    vec4 pos_ws; // Light position in world space.
    //----------------------------------- (16 byte boundary)
    vec4 pos_vs; // Light position in view space.
    //----------------------------------- (16 byte boundary)
    vec4 dir_ws; // Light direction in world space.
    //----------------------------------- (16 byte boundary)
    vec4 dir_vs; // Light direction in view space.
    //----------------------------------- (16 byte boundary)
    vec4 color;
    //----------------------------------- (16 byte boundary)
    float spot_angle;
    float const_att;
    float lin_att;
    float quad_att;
    //----------------------------------- (16 byte boundary)
    vec3 ambient;
    float Padding;
    //----------------------------------- (16 byte boundary)
    // Total:                              16 * 7 = 112 bytes
};

layout(set = 0, binding = 3) uniform SpotLightObject {
    SpotLight obj;
} spot_light;

struct DirLight {
    vec4 dir_ws;  // Light direction in world space.
    //----------------------------------- (16 byte boundary)
    vec4 dir_vs;  // Light direction in view space.
    //----------------------------------- (16 byte boundary)
    vec4 color;
    //----------------------------------- (16 byte boundary)
    vec3 ambient;
    float Padding;
    //----------------------------------- (16 byte boundary)
    // Total:                              16 * 4 = 64 bytes
};

layout(set = 0, binding = 4) uniform DirLightObject {
    DirLight obj;
} directional_light;

layout(set = 0, binding = 5) uniform LightPropsObject {
    uint num_point_lights;
    uint num_spot_lights;
    uint num_directional_lights;
} light_properties;

struct LightResult {
    vec4 diffuse;
    vec4 specular;
    vec4 ambient;
};

struct BlinnPhongSpecMaterial {
    vec3 fresnelR0;
    float shininess;
};

layout(set = 0, binding = 6) uniform FogPropsObject {
    vec4 color;
    float start;
    float range;
} fog_properties;

layout(location = 0) in vec4 pos_vs;
layout(location = 1) in vec3 normal_vs;
layout(location = 2) in vec3 tangent_vs;
layout(location = 3) in vec3 bitangent_vs;
layout(location = 4) in vec2 uv_ts;
layout(location = 5) in vec4 shadow_hs;

layout(location = 0) out vec4 outColor;

float DoDiffuse(vec3 n, vec3 L) {
    return max(0.0f, dot(n, L));
}

float DoSpecular(vec3 v, vec3 n, vec3 L, float specular_power) {
    vec3 r = normalize(reflect(-L, n));
    float r_dot_v = max(0.0f, dot(r, v));

    return pow(r_dot_v, specular_power);
}

float CalcLinearAttenuation(float d, float falloff_start, float falloff_end) {
    // Linear falloff.
    return clamp((falloff_end - d) / (falloff_end - falloff_start), 0.0f, 1.0f);
}

float DoAttenuation(float c, float l, float q, float d) {
    return 1.0f / ( c + l * d + q * d * d );
}

float DoSpotCone(vec3 spot_dir, vec3 L, float spot_angle) {
    float min_cos = cos(spot_angle);
    float max_cos = (min_cos + 1.0f) / 2.0f;
    float cos_angle = dot(spot_dir, -L);
    return smoothstep(min_cos, max_cos, cos_angle);
}

// Schlick gives an approximation to Fresnel reflectance (see pg. 233 "Real-Time Rendering 3rd Ed.").
// R0 = ( (n-1)/(n+1) )^2, where n is the index of refraction.
vec3 SchlickFresnel(vec3 R0, vec3 normal, vec3 light_direction_normal_ws) {
    float cos_incident_angle = clamp(dot(normal, light_direction_normal_ws), 0.0f, 1.0f);

    float f0 = 1.0f - cos_incident_angle;
    vec3 reflect_percent = R0 + (1.0f - R0) * (f0 * f0 * f0 * f0 * f0);

    return reflect_percent;
}

vec3 BlinnPhongSpec(vec3 light_direction_normal_ws, vec3 normal, vec3 to_eye, BlinnPhongSpecMaterial mat) {
    const float m = mat.shininess;
    vec3 half_vec = normalize(to_eye + light_direction_normal_ws);

    float roughness_factor = (m + 8.0f) * pow(max(dot(half_vec, normal), 0.0f), m) / 8.0f;
    vec3 fresnel_factor = SchlickFresnel(mat.fresnelR0, half_vec, light_direction_normal_ws);

    vec3 spec_albedo = fresnel_factor * roughness_factor;

    // Our spec formula goes outside [0,1] range, but we are 
    // doing LDR rendering.  So scale it down a bit.
    spec_albedo = spec_albedo / (spec_albedo + 1.0f);

    return spec_albedo;
}

LightResult DoPointLightVS(PointLight light, BlinnPhongSpecMaterial pbr, vec3 v, vec3 p, vec3 n) {
    LightResult result;
 
    vec3 L = (light.pos_vs.xyz - p);
    float d = length(L);
    L = L / d;
    float attenuation = DoAttenuation(light.const_att, light.lin_att, light.quad_att, d);
    result.diffuse = light.color * DoDiffuse(n, L) * attenuation;
    //result.Specular = light.Color * DoSpecular(V, N, L, pbr.Shininess) * attenuation;
    result.specular = light.color * vec4(BlinnPhongSpec(L, n, v, pbr), 0.0f) * attenuation;
    result.ambient = light.color * vec4(light.ambient, 0.0f);
    return result;
}

LightResult DoSpotLightVS(SpotLight light, BlinnPhongSpecMaterial pbr, vec3 v, vec3 p, vec3 n) {
    LightResult result;
 
    vec3 L = (light.pos_vs.xyz - p);
    float d = length(L);
    L = L / d;
    float attenuation = DoAttenuation(light.const_att, light.lin_att, light.quad_att, d);
    float spot_intensity = DoSpotCone(light.dir_vs.xyz, L, light.spot_angle);
    result.diffuse = light.color * DoDiffuse(n, L) * attenuation * spot_intensity;
    //result.Specular = DoSpecular(V, N, L, pbr.Shininess) * attenuation * spot_intensity * light.Color;
    result.specular = light.color * vec4(BlinnPhongSpec(L, n, v, pbr), 0.0f) * attenuation * spot_intensity;
    result.ambient = light.color * vec4(light.ambient, 0.0f);
    return result;
}

LightResult DoDirectionalLightVS(DirLight light, BlinnPhongSpecMaterial pbr, vec3 v, vec3 p, vec3 n) {
    LightResult result;
    vec3 L = normalize(-light.dir_vs.xyz);
    result.diffuse = light.color * DoDiffuse(n, L);
    //result.Specular = light.Color * DoSpecular(v, n, L, pbr.Shininess);
    result.specular = light.color * vec4(BlinnPhongSpec(L, n, v, pbr), 0.0f);
    result.ambient = light.color * vec4(light.ambient, 0.0f);
    return result;
}

LightResult DoLighting(BlinnPhongSpecMaterial pbr, vec3 p, vec3 n) {
    uint i;
    // Lighting is performed in view space.
    vec3 v = normalize(-p);
    LightResult total_result;
    total_result.diffuse = vec4(0.0f, 0.0f, 0.0f, 0.0f);
    total_result.specular = vec4(0.0f, 0.0f, 0.0f, 0.0f);
    total_result.ambient = vec4(0.0f, 0.0f, 0.0f, 0.0f);

    // Iterate point lights.
    if (light_properties.num_point_lights > 0) {
        LightResult result = DoPointLightVS(point_light.obj, pbr, v, p, n);
        total_result.diffuse += result.diffuse;
        total_result.specular += result.specular;
        total_result.ambient += result.ambient;
    }
    // Iterate spot lights.
    if (light_properties.num_spot_lights > 0) {
        LightResult result = DoSpotLightVS(spot_light.obj, pbr, v, p, n);
        total_result.diffuse += result.diffuse;
        total_result.specular += result.specular;
        total_result.ambient += result.ambient;
    }
    // Iterate directinal lights
    if (light_properties.num_directional_lights > 0) {
        LightResult result = DoDirectionalLightVS(directional_light.obj, pbr, v, p, n);
        total_result.diffuse += result.diffuse;
        total_result.specular += result.specular;
        total_result.ambient += result.ambient;
    }
    total_result.diffuse = clamp(total_result.diffuse, 0.0, 1.0f);
    total_result.specular = clamp(total_result.specular, 0.0, 1.0f);
    total_result.ambient = clamp(total_result.ambient, 0.0, 1.0f);
    return total_result;
}

layout(set = 0, binding = 7) uniform sampler2D texture_sampler;
layout(set = 0, binding = 8) uniform sampler2D shadow_sampler;
layout(set = 0, binding = 9) uniform sampler2D normal_sampler;
layout(set = 0, binding = 10) uniform sampler2D bump_sampler;
layout(set = 0, binding = 11) uniform sampler2D opacity_sampler;
layout(set = 0, binding = 12) uniform sampler2D ambient_sampler;
layout(set = 0, binding = 13) uniform sampler2D emissive_sampler;
layout(set = 0, binding = 14) uniform sampler2D specular_sampler;
layout(set = 0, binding = 15) uniform sampler2D metalness_sampler;

vec3 ExpandNormal(vec3 n) {
	return n * 2.0f - 1.0f;
}

vec3 DoNormalMapping(mat3 TBN, vec2 uv) {
	vec3 N = texture(normal_sampler, uv).xyz;
	N = ExpandNormal(N);

    // Transform normal from tangent space to view space.
	N = N * TBN;
	return normalize(N);
}

vec3 DoBumpMapping(mat3 TBN, vec2 uv, float bump_scale) {
    // Sample the heightmap at the current texture coordinate.
	float height_00 = texture(texture_sampler, uv).r * bump_scale;
    // Sample the heightmap in the U texture coordinate direction.
	float height_10 = textureOffset(texture_sampler, uv, ivec2(1, 0)).r * bump_scale;
    // Sample the heightmap in the V texture coordinate direction.
	float height_01 = textureOffset(texture_sampler, uv, ivec2(0, 1)).r * bump_scale;

	vec3 p_00 = vec3(0, 0, height_00);
	vec3 p_10 = vec3(1, 0, height_10);
	vec3 p_01 = vec3(0, 1, height_01);

    // normal = tangent x bitangent
	vec3 tangent = normalize(p_10 - p_00);
	vec3 bitangent = normalize(p_01 - p_00);

	vec3 normal = cross(tangent, bitangent);

    // Transform normal from tangent space to view space.
	normal = normal * TBN;

	return normal;
}


// If c is not black, then blend the color with the texture
// otherwise, replace the color with the texture.
vec4 SampleTexture(vec2 uv, vec4 c) {
	if (any(bvec3(c.r > 0.0f, c.g > 0.0f, c.b > 0.0f))) {
		c *= texture(texture_sampler, uv);
	}
	else {
		c = texture(texture_sampler, uv);
	}

	return c;
}

float CalcShadowFactor(vec4 shadow_pos_hs) {
    // Complete projection by doing division by w.
    shadow_pos_hs.xyz /= shadow_pos_hs.w;

    // Depth in NDC space.
    float depth = shadow_pos_hs.z;

    uint numMips = textureQueryLevels(shadow_sampler);
    ivec2 tex_size = textureSize(shadow_sampler, 0);
    uint width = tex_size.x;
    uint height = tex_size.y;

    // Texel size.
    float dx = 1.0f / float(width);

    float percent_lit = 0.0f;
    const vec2 offsets[9] = {
        vec2(-dx,  -dx), vec2(0.0f,  -dx), vec2(dx,  -dx),
        vec2(-dx, 0.0f), vec2(0.0f, 0.0f), vec2(dx, 0.0f),
        vec2(-dx,  +dx), vec2(0.0f,  +dx), vec2(dx,  +dx)
    };

    for(int i = 0; i < 9; ++i) {
        percent_lit += texture(shadow_sampler, shadow_pos_hs.xy + offsets[i], depth).r;
    }
    
    return percent_lit / 9.0f;
}

void main() {
    const uint HAS_AMBIENT_TEXTURE        = 1u;
	const uint HAS_EMISSIVE_TEXTURE       = 2u;
	const uint HAS_DIFFUSE_TEXTURE        = 4u;
	const uint HAS_SPECULAR_TEXTURE       = 8u;
	const uint HAS_SPECULAR_POWER_TEXTURE = 16u; // aiTextureType_SHININESS->roughness
	const uint HAS_NORMAL_TEXTURE         = 32u;
	const uint HAS_BUMP_TEXTURE           = 64u;
	const uint HAS_OPACITY_TEXTURE        = 128u;
	const uint HAS_DISPLACEMENT_TEXTURE   = 256u;
    const uint HAS_METALNESS_TEXTURE      = 512u;
    const uint HAS_SHADOW_TEXTURE         = 1024u;
    
    vec2 texture_uv = uv_ts.xy;

    // By default, use the alpha component of the diffuse color.
	float alpha = material.diffuse.a;
	if ((material.has_texture & HAS_OPACITY_TEXTURE) > 0) {
		alpha = texture(opacity_sampler, texture_uv).r;
	}

    if (alpha < 0.1f) {
        discard; // Discard the pixel if it is below a certain threshold.
    }

	vec4 ambient = material.ambient;
	if ((material.has_texture & HAS_AMBIENT_TEXTURE) > 0) {
		ambient = texture(ambient_sampler, texture_uv);
	}
    
    vec4 emissive = material.emissive;
	if ((material.has_texture & HAS_EMISSIVE_TEXTURE) > 0) {
		emissive = texture(emissive_sampler, texture_uv);
	}
    
    vec4 diffuse_albedo = material.diffuse;
	if ((material.has_texture & HAS_DIFFUSE_TEXTURE) > 0) {
		diffuse_albedo = texture(texture_sampler, texture_uv);
	}
    
    float specular_power = material.specular_power;
	if ((material.has_texture & HAS_SPECULAR_POWER_TEXTURE) > 0) {
		specular_power *= texture(specular_sampler, texture_uv).r;
	}
    
    float k = (material.ior - 1.0f) / (material.ior + 1.0f);
    float k2 = k * k;
    vec3 fresnelR0 = max(vec4(k2, k2, k2, k2), material.reflectance).xyz;
    if ((material.has_texture & HAS_METALNESS_TEXTURE) > 0) {
        vec3 metalness_sample = texture(metalness_sampler, texture_uv).rgb;
		fresnelR0 = mix(fresnelR0, diffuse_albedo.rgb, metalness_sample);
	}

	vec3 n;
    // Normal mapping
	if ((material.has_texture & HAS_NORMAL_TEXTURE) > 0) {
		vec3 tangent = normalize(tangent_vs);
		vec3 bitangent = normalize(bitangent_vs);
		vec3 normal = normalize(normal_vs);

		mat3 TBN = mat3(tangent, bitangent, normal);

		n = DoNormalMapping(TBN, texture_uv);
	}
	else if ((material.has_texture & HAS_BUMP_TEXTURE) > 0) {
		vec3 tangent = normalize(tangent_vs);
		vec3 bitangent = normalize(bitangent_vs);
		vec3 normal = normalize(normal_vs);

		mat3 TBN = mat3(tangent, bitangent, normal);

		n = DoBumpMapping(TBN, texture_uv, material.bump_intensity);
	}
	else {
		n = normalize(normal_vs);
	}

	float shadow = 1.0f;
    if ((material.has_texture & HAS_SHADOW_TEXTURE) > 0) {
        shadow = CalcShadowFactor(shadow_hs);
	}
    
	vec4 specular = vec4(0.0f, 0.0f, 0.0f, 0.0f);

	BlinnPhongSpecMaterial pbr;
    pbr.fresnelR0 = fresnelR0;
    pbr.shininess = specular_power;
    
    LightResult lit = DoLighting(pbr, pos_vs.xyz, n);
    ambient = ambient * 0.5f + diffuse_albedo * lit.ambient * 0.5f;
    diffuse_albedo *= lit.diffuse;
    // Specular power less than 1 doesn't really make sense.
    // Ignore specular on materials with a specular power less than 1.
    if (material.specular_power > 1.0f) {
        specular = material.specular;
        if ((material.has_texture & HAS_SPECULAR_TEXTURE) > 0) {
            specular = texture(specular_sampler, texture_uv);
        }
        specular *= lit.specular;
    }
    
    //float3 eye_position_w = IN.PositionVS - IN.PositionWS;
    //float3 to_eye = eye_position_w - IN.PositionWS.xyz;
    //float distance_to_eye = length(to_eye);
    float distance_to_eye = length(pos_vs.xyz);

    vec4 result = vec4(ambient.rgb + (emissive + diffuse_albedo + specular).rgb * shadow, alpha * material.opacity);
    
    float fog_start = fog_properties.start;
    float fog_range = fog_properties.range;
    vec3 fog_color = fog_properties.color.xyz;
    float fog_lerp = clamp((distance_to_eye - fog_start) / fog_range, 0.0f, 1.0f);
    result = vec4(mix(result.xyz, fog_color, fog_lerp), result.w);
    
    outColor = result;
	//outColor = vec4(ambient.rgb + (emissive + diffuse_albedo + specular).rgb * shadow, alpha * material.Opacity);
}