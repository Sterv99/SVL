#version 450
#extension GL_ARB_separate_shader_objects : enable

#define NR_POINT_LIGHTS 4
struct PointLight
{
    vec4 position;
    vec4 color;
    vec4 params;
};

layout(set = 0, binding = 0) uniform UniformBufferObject {
	mat4 model;
	mat4 view;
	mat4 proj;
	vec4 view_pos;
	PointLight point_light[NR_POINT_LIGHTS];
} ubo;

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec3 in_color;
layout(location = 2) in vec2 in_uv;
layout(location = 3) in vec3 in_normal;
layout(location = 4) in vec3 in_tangent;

layout(location = 0) out vec3 out_color;
layout(location = 1) out vec2 out_uv;
layout(location = 2) out vec3 out_normal;
layout(location = 3) out vec3 out_world_pos;
layout(location = 4) out mat3 TBN;


out gl_PerVertex {
	vec4 gl_Position;
};



void main()
{
    out_world_pos = vec3(ubo.model * vec4(in_pos, 1.0));
    //loc_pos.y = -loc_pos.y;

    out_color = in_color;
    out_uv = in_uv;
    out_normal = mat3(ubo.model) * in_normal;

    vec3 N = normalize(out_normal);
    vec3 T = normalize(mat3(ubo.model) * in_tangent);
    
    //T = normalize(T - dot(T, N) * N);
    vec3 B = normalize(cross(N, T));
    TBN = mat3(T, B, N);

    

    gl_Position = ubo.proj * ubo.view * vec4(out_world_pos, 1.0);
}
