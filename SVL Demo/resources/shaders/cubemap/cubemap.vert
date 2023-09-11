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

layout(location = 0) out vec3 out_uv;


out gl_PerVertex {
	vec4 gl_Position;
};

void main() 
{
    out_uv = in_pos;
    //out_uv.yz *= -1.0f;
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(in_pos, 1.0);
}