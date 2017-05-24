#version 450 core
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

struct Material
{
	uint albedoAlphaCompress;
	uint specRoughnessCompress;

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
	vec3 albedo;
	uint albedoAlphaMapIndex = u_material.material[inMaterialID].mapIndex & 0x7ff;
	if (albedoAlphaMapIndex == 0)
	{
		uint albedoAlphaCompress = u_material.material[inMaterialID].albedoAlphaCompress;
		albedo = vec3((albedoAlphaCompress & 0xff) / 255.0, ((albedoAlphaCompress >> 8) & 0xff) / 255.0, ((albedoAlphaCompress >> 16) & 0xff) / 255.0);
	}
	else
	{
		albedo = texture(mapSamplers[albedoAlphaMapIndex - 1], inTexcoord).rgb;
	}
	vec3 normal = inNormal;
	uint normalHeightMapIndex = (u_material.material[inMaterialID].mapIndex >> 10) & 0x7ff;
	if (normalHeightMapIndex != 0)
	{
		vec3 tn = normalize(texture(mapSamplers[normalHeightMapIndex - 1], inTexcoord).xyz * 2.0 - 1.0);
		normal = normalize(mat3(-inTangent, cross(normal, -inTangent), normal) * tn);
		//albedo = normal;
	}
	outAlbedoSpec = vec4(albedo, 0.5);
	outNormalRoughness = vec4(normal * 0.5 + 0.5, 0.5);
}