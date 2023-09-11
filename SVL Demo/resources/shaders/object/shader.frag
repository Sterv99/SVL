#version 450
#extension GL_ARB_separate_shader_objects : enable

#define NR_POINT_LIGHTS 4
struct PointLight
{
    vec4 position;
    vec4 color;
    vec4 params;
};

layout(set = 0, binding = 1) uniform UniformBufferObject {
	mat4 model;
	mat4 view;
	mat4 proj;
	vec4 view_pos;
    PointLight point_light[NR_POINT_LIGHTS];
} ubo;

layout(location = 0) in vec3 in_color;
layout(location = 1) in vec2 in_uv;
layout(location = 2) in vec3 in_normal;
layout(location = 3) in vec3 in_world_pos;
layout(location = 4) in mat3 TBN;

layout(location = 0) out vec4 out_color;

layout(set = 1, binding = 0) uniform MaterialProperties
{
    bool has_normal_tex;
    uint has_ao_tex;
    bool has_height_tex;
} mat;

layout(set = 1, binding = 1) uniform sampler2D texture_color;
layout(set = 1, binding = 2) uniform sampler2D texture_normal;
layout(set = 1, binding = 3) uniform sampler2D texture_metal_rough;
layout(set = 1, binding = 4) uniform sampler2D texture_ao;
layout(set = 1, binding = 5) uniform samplerCube texture_env;
layout(set = 1, binding = 6) uniform sampler2D texture_height;

const float PI = 3.14159265359;
const uint NUM_SAMPLES = 16;
const float GOLDEN_RATIO = 0.618034;

vec3 get_normal(vec2 uv)
{
    if(mat.has_normal_tex)
        return normalize(TBN * (texture(texture_normal, uv).rgb * 2.0 - 1.0));
    else
        return normalize(in_normal);
}

float get_ao()
{
    if(mat.has_ao_tex == 1)
        return texture(texture_metal_rough, in_uv).r;
    else if(mat.has_ao_tex == 2)
        return texture(texture_ao, in_uv).r;
    else
        return 1.0f;
}

vec2 get_uv(vec3 V)
{
    return in_uv;
}


vec3 F_SchlickR(float cosTheta, vec3 F0, float roughness)
{
	return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 F_Schlick_opt(vec3 F0, float VdotH)
{
    float ex = (-5.55473 * VdotH - 6.98316) * VdotH;
    return F0 + (1.0 - F0) * pow(2.0, ex);
}

float Dpl_GGX_TrowbridgeReitz(float NdotH2, float roughness2)
{
    float denom = 1.0 + NdotH2 * (roughness2 - 1.0);
    return roughness2 / (4.0 * denom * denom);
}

float G_GGXSmith(float NdotV, float NdotL, float roughness2)
{
    float k = roughness2 / 2.0;
    float nk = 1.0 - k;
    
    float g1 = NdotL / (NdotL * nk + k);
    float g2 = NdotV / (NdotV * nk + k);
    return g1 * g2;
}

float visibility(float NdotV, float NdotL, float roughness2)
{
    return G_GGXSmith(NdotV, NdotL, roughness2) / (NdotV * NdotL);
}



vec2 fibonacci_2D(uint i, uint N)
{
    return vec2(float(i+1) * GOLDEN_RATIO, (float(i)+0.5) / float(N));
}

vec3 importance_sample_GGX(vec2 Xi, float roughness, vec3 N)
{
    float a = roughness * roughness;

    float phi = 2 * PI * Xi.x;
    float cos_th = sqrt( (1.0 - Xi.y) / (1.0 + (a*a - 1.0) * Xi.y) );
    float sin_th = sqrt(1.0 - cos_th * cos_th);

    vec3 H;
    H.x = sin_th * cos(phi);
    H.y = sin_th * sin(phi);
    H.z = cos_th;

    vec3 up_vec = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 tan_x = normalize(cross(up_vec, N));
    vec3 tan_y = cross(N, tan_x);

    return tan_x * H.x + tan_y * H.y + N * H.z;
}

vec3 specular_ibl(vec3 spec, float roughness, vec3 N, vec3 V)
{
    vec3 sum = vec3(0.0);

    for(uint i = 0; i < NUM_SAMPLES; ++i)
    {
        vec2 Xi = fibonacci_2D(i, NUM_SAMPLES);
        vec3 H = importance_sample_GGX(Xi, roughness, N);
        vec3 L = normalize(2.0 * dot(V, H) * H - V);

        float NdotL = max(dot(N, L), 0.0);
        float NdotV = max(dot(N, V), 0.0001);
        float NdotH = max(dot(N, H), 0.0);
        float VdotH = max(dot(V, H), 0.0);

        if(NdotL > 0.0)
        {
            vec3 sample_color = texture(texture_env, L).rgb;

            float G = G_GGXSmith(NdotV, NdotL, roughness * roughness);
            float G_vis = G * VdotH / (NdotH * NdotV);
            float Fc = pow(1.0 - VdotH, 5.0);
            vec3 F = (1.0 - Fc) * spec + Fc;

            sum += sample_color * F * G_vis;
        }                   
    }

    return sum / float(NUM_SAMPLES);
}

vec3 get_specular(vec3 N, vec3 L, vec3 V, vec3 F0, float metallic, float roughness, vec3 albedo)
{
        //cook-torrance
        vec3 H = normalize(V + L);
        float NdotL = max(dot(N, L), 0.0);
        float NdotV = max(dot(N, V), 0.0);
        float NdotH = max(dot(N, H), 0.0);
        float VdotH = max(dot(V, H), 0.0);

        float denominator = 4.0 * NdotL * NdotV;

        vec3 F = F_Schlick_opt(F0, VdotH);
        float vis = visibility(NdotV, NdotL, roughness * roughness);
        float Dpl = Dpl_GGX_TrowbridgeReitz(NdotH*NdotH, roughness * roughness);//PI/4.0 * D;

        vec3 specular = F * max(vis, 0.001) * Dpl;
        //////////////

        vec3 kS = F; //reflection/specular
        vec3 kD = vec3(1.0) - kS; //reflection/diffuse
        kD *= 1.0 - metallic;

        vec3 Fr = kD * albedo / PI + specular;//Kd * f_lambert + Ks * f_cook-torrance

        return Fr * NdotL;
}

//metal B, rough G

void main()
{
    vec3 V = normalize(ubo.view_pos.xyz - in_world_pos);
    vec2 uv = get_uv(normalize(transpose(TBN) * V));

    vec3 albedo = pow(texture(texture_color, uv).rgb, vec3(2.2));
    float metallic = texture(texture_metal_rough, uv).b;
    float roughness = texture(texture_metal_rough, uv).g;
    float ao = get_ao();

    vec3 N = get_normal(uv);

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);
    
    vec3 Lo = vec3(0.0);
    for(int i = 0; i < NR_POINT_LIGHTS; i++)
    {
        if(ubo.point_light[i].color.a != 1.0f)
            continue;

        vec3 L = normalize(ubo.point_light[i].position.xyz - in_world_pos);

        float dist = length(L);
        float attenuation = pow(ubo.point_light[i].params.x / max(dist, 0.0001), 2.0);
        vec3 radiance = ubo.point_light[i].color.rgb * attenuation;
        
        Lo += get_specular(N, L, V, F0, metallic, roughness, albedo) * radiance;
    }
    
    vec3 F = F_SchlickR(clamp(dot(N, V), 0.0, 1.0), F0, roughness);
    
    vec3 diffuse = albedo;
    vec3 specular = specular_ibl(F, roughness, N, V);
    
    vec3 kS = F; //reflection/specular
    vec3 kD = vec3(1.0) - kS; //reflection/diffuse
    kD *= 1.0 - metallic;
    vec3 ibl = (kD * diffuse + specular) * ao;
    
    vec3 color = 0.04*albedo + Lo + ibl;

    //HDR tonemapping
    color = color / (color + vec3(1.0));
    //gamma correction
    color = pow(color, vec3(1.0/2.2));

    out_color = vec4(color, 1.0);
}
