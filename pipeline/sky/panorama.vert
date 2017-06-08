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

layout(location = 0) in vec3 inVertex;

layout(location = 0) out vec3 outNormal;

void main()
{
	gl_Position = u_matrix.projViewRotate * vec4(inVertex, 1);
	outNormal = inVertex;
}