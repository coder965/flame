#version 450 core
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

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