#include "terrain.h"

layout(binding = TKE_UBO_BINDING) uniform TERRAIN
{
	float ext;
	float height;
	float tessFactor;
	float mapDim;
}u_terrain;

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

layout(binding = TKE_UBO_BINDING) uniform sampler2D heightMap;

layout(binding = TKE_UBO_BINDING) uniform sampler2D colorMaps[4];

layout (location = 0) in vec2 inUV;

layout(location = 0) out vec4 outAlbedoAlpha;
layout(location = 1) out vec4 outNormalHeight;
layout(location = 2) out vec4 outSpecRoughness;

float getHeight(vec2 UV)
{
	return texture(heightMap, UV).r * u_terrain.height;
}

void main()
{
	mat3 normalMatrix = mat3(u_matrix.view);
	
	vec2 step = vec2(1.0 / u_terrain.mapDim, 0);
	float eps = (PATCH_SIZE * u_terrain.ext) * step.x;
	
	float L  = getHeight(inUV - step.xy);
	float R  = getHeight(inUV + step.xy);
	float T  = getHeight(inUV - step.yx);
	float B  = getHeight(inUV + step.yx);
	
	vec3 normal = normalMatrix * normalize(vec3(L - R, 2.0 * eps, T - B));

	vec3 color = vec3(0);
	float h = texture(heightMap, inUV).r;
	vec2 colUV = inUV * PATCH_SIZE * TEX_SIZE;
	if (h < 0.33)
	{
		float v = h / 0.33;
		color = mix(texture(colorMaps[0], colUV).rgb, texture(colorMaps[1], colUV).rgb, v);
	}
	else if (h < 0.66)
	{
		float v = (h - 0.33) / 0.33;
		color = mix(texture(colorMaps[1], colUV).rgb, texture(colorMaps[2], colUV).rgb, v);
	}
	else
	{
		float v = (h - 0.66) / 0.33;
		color = mix(texture(colorMaps[2], colUV).rgb, texture(colorMaps[3], colUV).rgb, v);
	}
	
	outAlbedoAlpha = vec4(color, 1.0);
	outNormalHeight = vec4(normal * 0.5 + 0.5, 0.0);
	outSpecRoughness = vec4(0.05, 1.0, 0.0, 0.0);
}
