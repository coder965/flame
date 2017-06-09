#include "..\depth.h"
#include "..\pi.h"
#include "..\panorama.h"

#if !defined(USE_PHGON) && !defined(USE_PBR)
#define USE_PHONG
#endif

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
	vec4 spotData;
};

layout(binding = 3) uniform LIGHT
{
	uint count;
	Light lights[256];
}u_light;
      
layout(binding = 4) uniform sampler2D depthSampler;
layout(binding = 5) uniform sampler2D albedoSpecSampler;
layout(binding = 6) uniform sampler2D normalRoughnessSampler;
layout(binding = 7) uniform sampler2D aoSampler;
#if defined(USE_IBL)
layout(binding = 8) uniform sampler2D envrSampler;
#endif

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
	return r * r * PI_INV;
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

		vec3 lightDir = light.coord.xyz;;
		if (light.coord.w == 0.0)
		{
			lightDir = (u_matrix.view * vec4(lightDir, 0.0)).xyz;
		}
		else
		{
			lightDir = (u_matrix.view * vec4(lightDir, 1.0)).xyz - coordView;
			float dist = length(lightDir);
			lightColor /= (dist * (dist * light.decayFactor.x + light.decayFactor.y) + light.decayFactor.z);
		}
		lightDir = normalize(lightDir);
		float nl = dot(normal, lightDir);
		if (nl < 0.0) continue;

		if (light.coord.w == 2.0)
		{
			float spotVal = dot(-lightDir, vec3(u_matrix.view * vec4(light.spotData.xyz, 0.0)));
			spotVal -= light.spotData.w;
			if (spotVal < 0.0) continue;
			float k = spotVal / (1.0 - light.spotData.w);
			lightColor *= k * k;
		}
#if defined(USE_PHONG)
		vec3 r = reflect(-lightDir, normal);
		float vr = max(dot(r, -viewDir), 0.0);
		lightSumColor += albedo * lightColor * nl + spec * pow(vr, (1.0 - roughness) * 128.0) * lightColor;
#elif defined(USE_PBR)
		lightSumColor += brdf(-viewDir, lightDir, normal, roughness, spec, albedo, lightColor) * nl;
#endif
	}
	
	vec3 color = lightSumColor;
#if defined(USE_IBL)
	float envrMipmaps = u_ambient.v.a;
	mat3 matrixViewInv3 = mat3(u_matrix.viewInv);
	vec3 irradiance = albedo * textureLod(envrSampler, panorama(matrixViewInv3 * normal), envrMipmaps).rgb;
	vec3 radiance = smothness * F_schlick(spec, dotNV) * textureLod(envrSampler, panorama(matrixViewInv3 * reflect(viewDir, normal)), roughness * envrMipmaps).rgb;
	color += litColor + u_ambient.v.rgb * (irradiance + radiance);
#else
	color += u_ambient.v.rgb * albedo;
#endif
	
	float fog = clamp(exp2( -0.01 * 0.01 * linerDepth * linerDepth * 1.442695), 0.0, 1.0);
	outColor = vec4(mix(u_ambient.fogColor.rgb, color, fog), 1.0);
	outColor = vec4(inAlbedoSpec.rgb, 1.0);
}
