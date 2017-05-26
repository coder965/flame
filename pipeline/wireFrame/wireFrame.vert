#version 450 core
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(push_constant) uniform PushConstant
{
	uint passIndex;
	vec4 color;
}pc;
layout(binding = 0) uniform MATRIX
{
	mat4 proj;
	mat4 projInv;
	mat4 view;
	mat4 viewInv;
	mat4 projView;
	mat4 projViewRotate;
	vec4 frustumPlanes[6];
	vec2 viewportDim;
}u_matrix;

layout(binding = 1) uniform INSTANCE
{
	mat4 matrix[1024];
}u_instance[2];

layout(location = 0) in vec3 inVertex;

void main()
{
	gl_Position = u_matrix.projView * u_instance[pc.passIndex].matrix[gl_InstanceIndex] * vec4(inVertex, 1);
}