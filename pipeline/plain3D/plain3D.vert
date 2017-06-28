layout(push_constant) uniform PushConstant
{
	mat4 modelMatrix;
	vec4 color;
}pc;

layout(binding = TKE_UBO_BINDING) uniform MATRIX
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
		
void main()
{
	gl_Position = u_matrix.projView * pc.modelMatrix * vec4(inVertex, 1);
}