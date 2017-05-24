#version 450 core
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(binding = 0) uniform sampler2D tex;

layout(binding = 1) uniform ALPHA
{
	float v;
}u_alpha;

layout(location = 0) in vec2 inTexcoord;

layout(location = 0) out vec4 outColor;
		
void main()
{
	outColor = vec4(texture(tex, inTexcoord).rgb, u_alpha.v);
}