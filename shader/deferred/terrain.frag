#version 450 core
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#include "..\debug.h"

layout(binding = 4) uniform TERRAIN
{
	vec3 coord;
	int blockCx;
	float blockSize;
	float height;
	float tessellationFactor;
	float textureUvFactor;
	float mapDimension;
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

layout(binding = 17) uniform sampler2D heightMap;
layout(binding = 18) uniform sampler2D normalMap;
layout(binding = 19) uniform sampler2D blendMap;
layout(binding = 20) uniform sampler2D colorMaps[4];
layout(binding = 21) uniform sampler2D normalMaps[4];

layout (location = 0) in vec2 inUV;

layout(location = 0) out vec4 outAlbedoAlpha;
layout(location = 1) out vec4 outNormalHeight;
layout(location = 2) out vec4 outSpecRoughness;

float getHeight(vec2 UV)
{
	return texture(heightMap, UV).r * u_terrain.height;
}

/*
mat3 rotate_matrix(float angle, vec3 axis)
{
	float c = cos(angle);
	float s = sin(angle);
	vec3 temp((1.0 - c) * axis);

	mat3 Rotate;
	Rotate[0][0] = c + temp[0] * axis[0];
	Rotate[0][1] = temp[0] * axis[1] + s * axis[2];
	Rotate[0][2] = temp[0] * axis[2] - s * axis[1];

	Rotate[1][0] = temp[1] * axis[0] - s * axis[2];
	Rotate[1][1] = c + temp[1] * axis[1];
	Rotate[1][2] = temp[1] * axis[2] + s * axis[0];

	Rotate[2][0] = temp[2] * axis[0] + s * axis[1];
	Rotate[2][1] = temp[2] * axis[1] - s * axis[0];
	Rotate[2][2] = c + temp[2] * axis[2];

	return Rotate;
}
*/

void main()
{
	mat3 normalMatrix = mat3(u_matrix.view);

	vec4 blend;
	blend.rgb = texture(blendMap, inUV).rgb;
	blend.a = 1.0 - blend.x - blend.y - blend.z;
	vec2 tilledUV = inUV * u_terrain.blockCx * u_terrain.textureUvFactor;

	vec3 color = vec3(0);
	color += texture(colorMaps[0], tilledUV).rgb * blend.r;
	color += texture(colorMaps[1], tilledUV).rgb * blend.g;
	color += texture(colorMaps[2], tilledUV).rgb * blend.b;
	color += texture(colorMaps[3], tilledUV).rgb * blend.a;
	/*
	vec3 normal = vec3(0);
	normal += (texture(normalMaps[0], tilledUV).xyz * 2.0 - 1.0) * blend.r;
	normal += (texture(normalMaps[1], tilledUV).xyz * 2.0 - 1.0) * blend.g;
	normal += (texture(normalMaps[2], tilledUV).xyz * 2.0 - 1.0) * blend.b;
	normal += (texture(normalMaps[3], tilledUV).xyz * 2.0 - 1.0) * blend.a;
	
	float h = texture(heightMap, inUV).r;
	vec2 step = vec2(1.0 / u_terrain.mapDimension, 0);
	float eps = (u_terrain.blockCx * u_terrain.blockSize) * step.x * 2.0;
	float LR  = getHeight(inUV - step.xy) - getHeight(inUV + step.xy);
	float TB  = getHeight(inUV - step.yx) - getHeight(inUV + step.yx);
	*/
	
	vec3 samNormal = texture(normalMap, inUV).xyz * 2.0 - 1.0;
	vec3 normal = normalMatrix * vec3(-samNormal.x, samNormal.z, -samNormal.y);

	outAlbedoAlpha = vec4(color, 1.0);
	outNormalHeight = vec4(normal * 0.5 + 0.5, 0.0);
	outSpecRoughness = vec4(0.05, 1.0, 0.0, 0.0);
}
