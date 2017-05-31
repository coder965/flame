#version 450 core
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

struct Material
{
	uint albedoSpecCompress;
	uint roughnessAlphaCompress;

	uint mapIndex;
	
	uint dummy;
};

layout(binding = 2) uniform MATERIAL
{
	Material material[1024];
}u_material;

layout(binding = 3) uniform sampler2D mapSamplers[1024];

layout(location = 0) in flat uint inMaterialID;
layout(location = 1) in vec2 inTexcoord;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec3 inTangent;

layout(location = 0) out vec4 outAlbedoSpec;
layout(location = 1) out vec4 outNormalRoughness;
		
void main()
{
	uint mapIndex;

	vec3 albedo;
	float spec, roughness;
	
	mapIndex = u_material.material[inMaterialID].mapIndex & 0x7ff;
	uint albedoSpecCompress = u_material.material[inMaterialID].albedoSpecCompress;
	albedo = vec3((albedoSpecCompress & 0xff) / 255.0, ((albedoSpecCompress >> 8) & 0xff) / 255.0, ((albedoSpecCompress >> 16) & 0xff) / 255.0);
	spec = ((albedoSpecCompress >> 24) & 0xff) / 255.0;
	if (mapIndex == 0)
	{
	}
	else
	{
		vec4 albedoSpec = texture(mapSamplers[mapIndex - 1], inTexcoord);
		albedo = albedoSpec.rgb;
		//spec = albedoSpec.a;
	}
	
	vec3 normal = inNormal;
	mapIndex = (u_material.material[inMaterialID].mapIndex >> 10) & 0x7ff;
	uint roughnessAlpha = u_material.material[inMaterialID].roughnessAlphaCompress;
	roughness = (roughnessAlpha & 0xff) / 255.0;
	if (mapIndex == 0)
	{
	}
	else
	{
		vec4 normalRoughness = texture(mapSamplers[mapIndex - 1], inTexcoord);
		vec3 tn = normalize(normalRoughness.xyz * 2.0 - 1.0);
		normal = normalize(mat3(-inTangent, cross(normal, -inTangent), normal) * tn);
		//roughness = normalRoughness.a;
	}
	
	outAlbedoSpec = vec4(albedo, spec);
	outNormalRoughness = vec4(normal * 0.5 + 0.5, roughness);
}