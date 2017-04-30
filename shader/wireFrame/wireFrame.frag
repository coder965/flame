layout(push_constant) uniform PushConstant
{
	layout (offset = 16) vec4 color;
}p_color;

layout(location = 0) out vec4 outColor;
		
void main()
{
	outColor = vec4(p_color.color);
}