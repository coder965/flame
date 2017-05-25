#version 450 core
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(binding = 0) uniform sampler2D originalSampler;

layout(location = 0) out vec4 outColor;

const float gamma = 0.45455;
//const float gamma = 1.0;

void main()
{
	outColor = vec4(pow(texture(originalSampler, gl_FragCoord.xy).rgb, vec3(gamma)), 1.0);
}
