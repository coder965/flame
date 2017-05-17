layout(binding = 0) uniform sampler2D tex;

layout(push_constant) uniform uPushConstant
{
	float alpha;
}pc;

layout(location = 0) in vec2 inTexcoord;

layout(location = 0) out vec4 outColor;
		
void main()
{
	outColor = vec4(texture(tex, inTexcoord).rgb, pc.alpha);
}