#include "..\esm.h"

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

struct Material
{
	uint albedoAlphaCompress;
	uint specRoughnessCompress;

	uint mapIndex;
	
	uint dummy;
};

layout(binding = 1) uniform MATERIAL
{
	Material material[256];
}u_material;

layout(set = 1, binding = 0) uniform sampler2D maps[256];

layout(location = 0) in vec2 inTexcoord;
layout(location = 1) in flat uint inMaterialID;

layout(location = 0) out float outExp;

highp float map_01(float x, float v0, float v1)
{
	return (x - v0) / (v1 - v0);
}
		
void main()
{
	/*
	uint mapIndex;

	float alpha;
	mapIndex = u_material.material[inMaterialID].mapIndex & 0xff;
	if (mapIndex == 0)
	{
		uint v = u_material.material[inMaterialID].albedoAlphaCompress;
		alpha = ((v >> 24) & 0xff) / 255.0;
	}
	else
	{
		vec4 v = texture(maps[mapIndex - 1], inTexcoord);
		alpha = v.a;
	}
	if (alpha < 0.5)
		discard;
	*/

	// Exponential is a configurable constant for approximation.
	// Generally a higher Exponential means greater difference in depths.
	// Because of this there will be less error, but we may run out of precision.
	outExp = exp(/*Light.Exponential*/esm_factor * (gl_FragCoord.z / gl_FragCoord.w));
	//outExp = gl_FragCoord.z / gl_FragCoord.w;
	//outExp = gl_FragCoord.z;
}