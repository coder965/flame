#version 450 core
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aUV;
layout(location = 2) in vec4 aColor;

layout(push_constant) uniform uPushConstant
{
	vec2 uScale;
	vec2 uTranslate;
}pc;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec2 outUV;
layout(location = 2) out flat uint outID;

void main()
{
	outColor = aColor;
	outUV = aUV;
	outID = gl_InstanceIndex;
	gl_Position = vec4(aPos * pc.uScale + pc.uTranslate, 0, 1);
}
