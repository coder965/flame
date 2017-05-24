#version 450 core
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(binding = 0) uniform MATRIX
{
	mat4 matrixProj;
	mat4 matrixProjInv;
	mat4 matrixView;
	mat4 matrixViewInv;
	mat4 matrixProjView;
	mat4 matrixProjViewRotate;
	vec4 frustumPlanes[6];
	vec2 viewportDim;
}u_matrix;

layout(location = 0) in vec3 inVertex;

layout(location = 0) out vec3 outNormal;

void main()
{
	gl_Position = u_matrix.matrixProjViewRotate * vec4(inVertex, 1);
	outNormal = inVertex;
}