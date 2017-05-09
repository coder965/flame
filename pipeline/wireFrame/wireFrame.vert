layout(push_constant) uniform PushConstant
{
	uint passIndex;
	vec4 color;
}pc;

layout(binding = 0) uniform MATRIX
{
	mat4 matrixProj;
	mat4 matrixProjInv;
	mat4 matrixView;
	mat4 matrixViewInv;
	mat4 matrixProjView;
	mat4 matrixProjViewRotate;
}u_matrix;

layout(binding = 1) uniform INSTANCE
{
	mat4 matrix[1024];
}u_instance[2];

layout(location = 0) in vec3 inVertex;

void main()
{
	gl_Position = u_matrix.matrixProjView * u_instance[pc.passIndex].matrix[gl_InstanceIndex] * vec4(inVertex, 1);
}