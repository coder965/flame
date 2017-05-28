#version 450 core
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
1
#include "..\pi.h"
#include "..\panorama.h"

layout(binding = 1) uniform sampler2D tex;
layout(location = 0) in vec3 inNormal;
layout(location = 0) out vec4 outColor;
		
void main()
{
	outColor = vec4(textureLod(tex, panorama(normalize(inNormal)), 0).rgb, 1.0);
}