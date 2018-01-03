#version 450 core
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#include "..\debug.h"

struct Terrain
{
	vec3 coord;
	int block_count;
	float block_size;
	float terrain_height;
	float displacement_height;
	float tessellation_factor;
	float tiling_scale;
	uint material_index;
};

layout(binding = 4) uniform TERRAIN
{
	Terrain d[8];
}u_terrain;

layout(binding = 2) uniform MATRIX
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

layout(binding = 18) uniform sampler2D blendMap[8];

layout (location = 0) in flat uint inTerrainId;
layout (location = 1) in vec2 inUV;
layout (location = 2) in vec3 inNormal;
layout (location = 3) in vec3 inTangent;

layout(location = 0) out vec4 outAlbedoAlpha;
layout(location = 1) out vec4 outNormalHeight;
layout(location = 2) out vec4 outSpecRoughness;

void main()
{
	mat3 normalMatrix = transpose(inverse(mat3(u_matrix.view));

	vec4 blend = texture(blendMap[inTerrainId], inUV);
	vec2 tilledUV = inUV * u_terrain.d[inTerrainId].block_count * u_terrain.d[inTerrainId].tiling_scale;

	vec3 color = vec3(0);
	color += texture(colorMaps[inTerrainId * 8 + 0], tilledUV).rgb * blend.r;
	color += texture(colorMaps[inTerrainId * 8 + 1], tilledUV).rgb * blend.g;
	color += texture(colorMaps[inTerrainId * 8 + 2], tilledUV).rgb * blend.b;
	color += texture(colorMaps[inTerrainId * 8 + 3], tilledUV).rgb * blend.a;

	outAlbedoAlpha = vec4(color, 1.0);
	outNormalHeight = vec4(inNormal * 0.5 + 0.5, 0.0);
	outSpecRoughness = vec4(0.05, 1.0, 0.0, 0.0);
}
