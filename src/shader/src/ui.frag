#version 450 core
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(binding = 0) uniform sampler2D images[128];

layout(location = 0) in vec4 inColor;
layout(location = 1) in vec2 inUV;
layout(location = 2) in flat uint inID;

layout(location = 0) out vec4 fColor;
 
void main()
{
	fColor = inColor * texture(images[inID], inUV);
	//fColor = vec4(1.0);
}