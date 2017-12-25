#version 450 core
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(push_constant) uniform PushConstant
{
	vec2 size;
}pc;

layout(binding = 0) uniform sampler2D source;

layout(location = 0) in vec2 inTexcoord;

layout(location = 0) out vec4 outColor;

void main()
{
	vec2 offset = 1.0 / pc.size;
	
	outColor = (
		texture(source, inTexcoord) +
		texture(source, inTexcoord + vec2(0.0, offset.y)) +
		texture(source, inTexcoord + vec2(offset.x, 0.0)) +
		texture(source, inTexcoord + vec2(offset.x, offset.y))
	) * 0.25;
}
