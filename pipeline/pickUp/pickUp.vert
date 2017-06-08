layout(push_constant) uniform PushConstant
{
	uint passIndex;
	uint index;
}p_index;

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

layout(location = 0) out flat uint outIndex;

void main()
{
	outIndex = p_index.index;
	gl_Position = u_matrix.projView * u_instance[p_index.passIndex].matrix[gl_InstanceIndex] * vec4(inVertex, 1);
}