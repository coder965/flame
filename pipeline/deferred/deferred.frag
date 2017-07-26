#include "..\depth.h"
#include "..\pi.h"
#include "..\panorama.h"

#if !defined(USE_PHGON) && !defined(USE_PBR)
#define USE_PHONG
#endif

layout(binding = TKE_UBO_BINDING) uniform CONSTANT
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

layout(binding = TKE_UBO_BINDING) uniform MATRIX
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

layout(binding = TKE_UBO_BINDING) uniform AMBIENT
{
	vec3 color;
	uint envr_max_mipmap;
	vec4 fogColor;
}u_ambient;

struct Light
{
	vec4 coord;
	vec4 color;
	vec4 spotData;
};

layout(binding = TKE_UBO_BINDING) uniform LIGHT
{
	uint count;
	Light lights[256];
}u_light;

layout(binding = TKE_UBO_BINDING) uniform SHADOW
{
	mat4 matrix[8];
}u_shadow;

layout(binding = TKE_UBO_BINDING) uniform sampler2D envrSampler;
layout(binding = TKE_UBO_BINDING) uniform sampler2D depthSampler;
layout(binding = TKE_UBO_BINDING) uniform sampler2D albedoAlphaSampler;
layout(binding = TKE_UBO_BINDING) uniform sampler2D normalHeightSampler;
layout(binding = TKE_UBO_BINDING) uniform sampler2D specRoughnessSampler;
layout(binding = TKE_UBO_BINDING) uniform sampler2D shadowSampler[48];
layout(binding = TKE_UBO_BINDING) uniform sampler2D aoSampler;

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

highp float map_01(float x, float v0, float v1)
{
	return (x - v0) / (v1 - v0);
}

void main()
{
	vec3 viewDir = normalize(inViewDir);

	float inDepth = texture(depthSampler, gl_FragCoord.xy).r;
	if (inDepth == 1.0)
	{
		outColor = vec4(textureLod(envrSampler, panorama(mat3(u_matrix.viewInv) * viewDir), 0).rgb, 1.0);
		return;
	}
		
	float linerDepth = LinearDepthPerspective(inDepth, u_constant.near, u_constant.far);
		
	vec3 coordView = inViewDir * (-linerDepth / inViewDir.z);
	vec4 coordWorld = u_matrix.viewInv * vec4(coordView, 1.0);
	
	vec4 inAlbedoAlpha = texture(albedoAlphaSampler, gl_FragCoord.xy);
	vec4 inNormalHeight = texture(normalHeightSampler, gl_FragCoord.xy);
	vec4 inSpecRoughness = texture(specRoughnessSampler, gl_FragCoord.xy);
	
	vec3 albedo = inAlbedoAlpha.rgb;
	float spec = inSpecRoughness.r;
	albedo *= 1.0 - spec;
	
	vec3 normal = normalize(inNormalHeight.xyz * 2.0 - 1.0);
	float roughness = inSpecRoughness.g;
	float smothness = 1.0 - roughness;
	
	vec3 lightSumColor = vec3(0.0);
	for (int i = 0; i < u_light.count; i++)
	{
		Light light = u_light.lights[i];

		float visibility = 1.0;
		{
			int shadowId = int(light.color.a);
			if (shadowId != -1)
			{
				vec4 shadowCoord = u_shadow.matrix[shadowId] * coordWorld;
				float occluder = textureProj(shadowSampler[shadowId * 6], shadowCoord.xyw).r;
				float reciever = map_01(shadowCoord.w, u_constant.near, u_constant.far);
				reciever = shadowCoord.z / shadowCoord.w;
				visibility = clamp(occluder * exp(-128.0 * reciever), 0.0, 1.0);
				visibility = occluder - reciever > 0.0 ? 0.0 : 1.0;
			}
		}
		if (visibility == 0.0) continue;
		
		vec3 lightColor = light.color.xyz * visibility;
		vec3 lightDir = light.coord.xyz;
		if (light.coord.w == 0.0)
		{
			lightDir = (u_matrix.view * vec4(lightDir, 0.0)).xyz;
		}
		else
		{
			lightDir = (u_matrix.view * vec4(lightDir, 1.0)).xyz - coordView;
			lightColor /= lightDir.x * lightDir.x + lightDir.y * lightDir.y + lightDir.z * lightDir.z;
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
	mat3 matrixViewInv3 = mat3(u_matrix.viewInv);
	vec3 irradiance = albedo * textureLod(envrSampler, panorama(matrixViewInv3 * normal), u_ambient.envr_max_mipmap).rgb;
	vec3 radiance = smothness * F_schlick(spec, dot(-viewDir, normal)) * textureLod(envrSampler, panorama(matrixViewInv3 * reflect(viewDir, normal)), roughness * u_ambient.envr_max_mipmap).rgb;
	color += u_ambient.color * (irradiance + radiance);
#else
	color += u_ambient.color * albedo;
#endif
	
	float fog = clamp(exp2(-0.01 * 0.01 * linerDepth * linerDepth * 1.442695), 0.0, 1.0);
	//outColor = vec4(mix(u_ambient.fogColor.rgb, color, fog), 1.0);
	outColor = vec4(mix(u_ambient.fogColor.rgb, color, 1), 1.0);
}
