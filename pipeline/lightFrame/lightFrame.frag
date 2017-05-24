#version 450 core
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(location = 0) in vec3 inNormal;

layout(location = 0) out vec4 outColor;
		
void main()
{
	outColor = vec4(vec3(dot(inNormal, vec3(0.0, 0.0, 1.0))), 1.0);
}