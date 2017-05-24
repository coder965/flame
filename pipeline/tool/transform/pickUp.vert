#version 450 core
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(binding = 0) uniform VERT_DATA
{
	mat4 matrixMVP[4];
	mat4 matrixNormal[4];
}u_data;

layout(location = 0) in vec3 inVertex;

layout(location = 0) out flat uint outInstanceIndex;

void main()
{
	outInstanceIndex = gl_InstanceIndex;
	gl_Position = u_data.matrixMVP[gl_InstanceIndex] * vec4(inVertex, 1.0);
}