#version 450 core
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#include "..\depth.h"
#include "..\pi.h"
#include "..\panorama.h"

layout(binding = 0) uniform MATRIX
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

layout(binding = 1) uniform AMBIENT
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

layout(binding = 2) uniform LIGHT
{
	uint count;
	Light lights[1024];
}u_light;
      
layout(binding = 3) uniform sampler2D depthSampler;
layout(binding = 4) uniform sampler2D albedoSpecSampler;
layout(binding = 5) uniform sampler2D normalRoughnessSampler;
layout(binding = 6) uniform sampler2D aoSampler;
layout(binding = 7) uniform sampler2D envrSampler;

layout(location = 0) in vec3 inViewDir;

layout(location = 0) out vec4 outColor;

float F_schlick(float f0, float cita)
{
	return f0 + (1.0 - f0) * pow(1.0 - cita, 5.0);
}

float G1_schlick(float cita, float k)
{
	return cita / (cita * (1.0 - k) + k);
}

float G_schlick(float alpha, float nl, float nv)
{
	float k = 0.8 + 0.5 * alpha;
	k *= k * 0.5;
	return G1_schlick(nl, k) * G1_schlick(nv, k);
}

float D_GGX(float alpha, float nh)
{
	float r = alpha / (nh * (alpha * alpha - 1.0) + 1.0);
	return r * r * INV_PI;
}

vec3 brdf(vec3 V, vec3 L, vec3 N, float roughness, float spec, vec3 albedo, vec3 lightColor)
{
	vec3 H = normalize(L + V);
	float nl = dot(N, L);
	float nv = dot(N, V);
	float alpha = pow(1.0 - (1.0 - roughness) * 0.7, 6.0);
	return (albedo + D_GGX(alpha, dot(N, H)) * F_schlick(spec, dot(L, H)) * G_schlick(alpha, nl, nv) / (4.0 * nl * nv)) * lightColor;
}

float specularOcclusion(float dotNV, float ao, float smothness)
{
    return clamp(pow(dotNV + ao, smothness) - 1.0 + ao, 0.0, 1.0);
}	

void main()
{
	float inDepth = texture(depthSampler, gl_FragCoord.xy).r * 2.0 - 1.0;
	float linerDepth = LinearDepthPerspective(inDepth);
	if (linerDepth > 999.0)
		discard;
		
	vec3 viewDir = normalize(inViewDir);
	vec3 coordView = viewDir * (-linerDepth / viewDir.z);
	
	vec4 inAlbedoSpec = texture(albedoSpecSampler, gl_FragCoord.xy);
	vec4 inNormalRoughness = texture(normalRoughnessSampler, gl_FragCoord.xy);
	
	vec3 albedo = inAlbedoSpec.rgb;
	float spec = inAlbedoSpec.a;
	albedo *= 1.0 - spec;
	
	vec3 normal = normalize(inNormalRoughness.xyz * 2.0 - 1.0);
	float roughness = inNormalRoughness.a;
	
	float dotNV = dot(normal, -viewDir);
	float smothness = 1.0 - roughness;
	
	float ao = 1.0;
	float specAo = specularOcclusion(dotNV, ao, smothness);
	
	vec3 litColor = vec3(0.0);
	for (int i = 0; i < u_light.count; i++)
	{
		Light light = u_light.lights[i];
		vec3 lightColor = light.color.rgb;
		vec4 lightCoord = u_matrix.view * light.coord;
		vec3 lightDir = lightCoord.xyz - coordView * lightCoord.w;
		if (light.coord.w == 1.0)
		{
			float dist = length(lightDir);
			//lightColor /= dist;
			lightColor /= vec3(dist);
			//lightColor *= 1.0 / (dist * (dist * light.decayFactor.x + light.decayFactor.y) + light.decayFactor.z);
		}
		lightDir = normalize(lightDir);
		float nl = dot(normal, lightDir);
		//litColor += brdf(-viewDir, lightDir, normal, roughness, spec, albedo, lightColor) * nl;
		//litColor += vec3(nl) * albedo * lightColor;
		litColor += lightColor;
	}
	
	vec3 color = vec3(litColor + u_ambient.v.rgb * albedo);
	//vec3 color = vec3(litColor);
	
	float fog = clamp(exp2( -0.01 * 0.01 * linerDepth * linerDepth * 1.442695), 0.0, 1.0);

	//outColor = vec4(mix(u_ambient.fogColor.rgb, color, fog), 1.0);
	//outColor = vec4(litColor, 1.0);
	outColor = vec4(1.0);
}
