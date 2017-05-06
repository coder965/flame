616layout(binding = 0) uniform MATRIX
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

layout(binding = 1) uniform LIGHT
{
	mat4 matrix[1024];
}u_light;

layout(location = 0) in vec3 inVertex;
layout(location = 2) in vec3 inNormal;

layout(location = 0) out vec3 outNormal;

void main()
{
	mat4 modelMatrix = u_light.matrix[gl_InstanceIndex];
	outNormal = mat3(u_matrix.matrixView * modelMatrix) * inNormal;
	gl_Position = u_matrix.matrixProjView * modelMatrix * vec4(inVertex, 1);
}