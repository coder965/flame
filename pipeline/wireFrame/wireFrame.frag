layout(push_constant) uniform PushConstant
{
	uint passIndex;
	vec4 color;
}pc;

layout(location = 0) out vec4 outColor;
		
void main()
{
	outColor = vec4(pc.color);
}