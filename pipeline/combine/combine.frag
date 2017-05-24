#version 450 core
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(binding = 0) uniform sampler2D originalSampler;
layout(binding = 1) uniform sampler2D miscSampler;

layout(location = 0) out vec4 outColor;

const float gamma = 0.45455;
//const float gamma = 1.0;

void main()
{
	vec4 miscColor = texture(miscSampler, gl_FragCoord.xy);
	outColor = vec4((pow(texture(originalSampler, gl_FragCoord.xy).rgb, vec3(gamma)) * (1.0 - miscColor.a) + miscColor.rgb), 1.0);
	//outColor = vec4(texture(originalSampler, gl_FragCoord.xy).rgb,1.0);
	//outColor = vec4(1.0);
}
