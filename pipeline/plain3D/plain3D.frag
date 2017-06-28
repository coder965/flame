layout(push_constant) uniform PushConstant
{
	mat4 modelMatrix;
	vec4 color;
}pc;

layout(location = 0) out vec4 outColor;
		
void main()
{
	outColor = pc.color;
}