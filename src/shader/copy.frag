#version 450 core
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(binding = 0) uniform sampler2D img_source;

layout(location = 0) in vec2 inTexcoord;

layout(location = 0) out vec4 outColor;

void main()
{
	outColor = texture(img_source, inTexcoord);
}
