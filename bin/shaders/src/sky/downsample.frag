layout(push_constant) uniform PushConstant
{
	vec2 size;
}pc;

layout(binding = 0) uniform sampler2D img_source;

layout(location = 0) in vec2 inTexcoord;

layout(location = 0) out vec4 outColor;

void main()
{
	vec2 offset = 1.0 / pc.size;
	
	outColor = (
		texture(img_source, inTexcoord) +
		texture(img_source, inTexcoord + vec2(0.0, offset.y)) +
		texture(img_source, inTexcoord + vec2(offset.x, 0.0)) +
		texture(img_source, inTexcoord + vec2(offset.x, offset.y))
	) * 0.25;
}
