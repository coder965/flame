#version 450 core
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aUV;

layout(location = 0) out vec2 outUV;

void main()
{
	outUV = aUV;
	gl_Position = vec4(aPos, 0, 1);
}