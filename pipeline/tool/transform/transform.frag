#version 450 core
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(binding = 1) uniform FRAG_DATA
{
	vec4 color[4];
}u_data;

layout(location = 0) in flat uint inInstanceIndex;
layout(location = 1) in vec3 inNormal;

layout(location = 0) out vec4 outColor;
		
void main()
{
	outColor = vec4(dot(inNormal, vec3(0.0, 0.0, 1.0)) * u_data.color[inInstanceIndex].rgb, 1.0);
}