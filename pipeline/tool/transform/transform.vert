layout(binding = 0) uniform VERT_DATA
{
	mat4 matrixMVP[4];
	mat4 matrixNormal[4];
}u_data;

layout(location = 0) in vec3 inVertex;
layout(location = 2) in vec3 inNormal;

layout(location = 0) out flat uint outInstanceIndex;
layout(location = 1) out vec3 outNormal;

void main()
{
	outInstanceIndex = gl_InstanceIndex;
	outNormal = mat3(u_data.matrixNormal[gl_InstanceIndex]) * inNormal;
	gl_Position = u_data.matrixMVP[gl_InstanceIndex] * vec4(inVertex, 1.0);
}