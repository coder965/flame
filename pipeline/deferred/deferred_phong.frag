#version 450 core
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#include "..\depth.h"
#include "..\pi.h"
#include "..\panorama.h"

layout(binding = 0) uniform CONSTANT
{
	float near;
	float far;
	float cx;
	float cy;
	float aspect;
	float fovy;
	float tanHfFovy;
	float envrCx;
	float envrCy;
}u_constant;

layout(binding = 1) uniform MATRIX
{
	mat4 proj;
	mat4 projInv;
	mat4 view;
	mat4 viewInv;
	mat4 projView;
	mat4 projViewRotate;
	vec4 frustumPlanes[6];
	vec2 viewportDim;
}u_matrix;

layout(binding = 2) uniform AMBIENT
{
	vec4 v; // (R,G,B) - ambient color, (A) - EnvrMipmaps
	vec4 fogColor;
}u_ambient;

struct Light
{
	vec4 coord;
	vec4 color;
	vec4 decayFactor;
};

layout(binding = 3) uniform LIGHT
{
	uint count;
	Light lights[1024];
}u_light;
      
layout(binding = 4) uniform sampler2D depthSampler;
layout(binding = 5) uniform sampler2D albedoSpecSampler;
layout(binding = 6) uniform sampler2D normalRoughnessSampler;
layout(binding = 7) uniform sampler2D aoSampler;
layout(binding = 8) uniform sampler2D envrSampler;

layout(location = 0) in vec3 inViewDir;

layout(location = 0) out vec4 outColor;

void main()
{
	float inDepth = texture(depthSampler, gl_FragCoord.xy).r;
	if (inDepth == 1.0)
		discard;
		
	float linerDepth = LinearDepthPerspective(inDepth, u_constant.near, u_constant.far);
		
	vec3 viewDir = normalize(inViewDir);
	vec3 coordView = inViewDir * (-linerDepth / inViewDir.z);
	
	vec4 inAlbedoSpec = texture(albedoSpecSampler, gl_FragCoord.xy);
	vec4 inNormalRoughness = texture(normalRoughnessSampler, gl_FragCoord.xy);
	
	vec3 albedo = inAlbedoSpec.rgb;
	float spec = inAlbedoSpec.a;
	albedo *= 1.0 - spec;
	
	vec3 normal = normalize(inNormalRoughness.xyz * 2.0 - 1.0);
	float roughness = inNormalRoughness.a;
	
	vec3 lightSumColor = vec3(0.0);
	for (int i = 0; i < u_light.count; i++)
	{
		Light light = u_light.lights[i];
		vec3 lightColor = light.color.xyz;
		vec4 lightCoord = u_matrix.view * light.coord;
		vec3 lightDir = lightCoord.xyz - coordView * lightCoord.w;
		if (light.coord.w == 1.0)
		{
			float dist = length(lightDir);
			lightColor /= (dist * (dist * light.decayFactor.x + light.decayFactor.y) + light.decayFactor.z);
		}
		lightDir = normalize(lightDir);
		float nl = max(dot(normal, lightDir), 0.0);
		if (nl > 0.0)
		{
			vec3 r = reflect(-lightDir, normal);
			float vr = max(dot(r, -viewDir), 0.0);
			lightSumColor += albedo * lightColor * nl;
			lightSumColor += spec * pow(vr, (1.0 - roughness) * 128.0) * lightColor;
		}
	}
	
	vec3 color = lightSumColor;
	color += u_ambient.v.rgb * albedo;
	
	float fog = clamp(exp2( -0.01 * 0.01 * linerDepth * linerDepth * 1.442695), 0.0, 1.0);

	outColor = vec4(mix(u_ambient.fogColor.rgb, color, fog), 1.0);
}
