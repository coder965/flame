#version 450 core
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(push_constant) uniform PushConstant
{
	mat4 modelview;
	mat4 proj;
	vec4 color;
}pc;

#if defined(USE_MATERIAL)
struct Material
{
	uint albedoAlphaCompress;
	uint specRoughnessCompress;

	uint mapIndex;
	
	uint dummy;
};

layout(set = 1, binding = 0) uniform ubo_material_
{
	Material material[256];
}ubo_material;

layout(set = 1, binding = 1) uniform sampler2D imgs_material[256];
#endif

#if defined(USE_MATERIAL)
layout(location = 0) in vec2 inTexcoord;
layout(location = 3) in flat uint inMaterialID;
#endif
#if defined(USE_NORMAL)
layout(location = 1) in vec3 inNormal;
#endif

layout(location = 0) out vec4 outColor;
		
void main()
{
	vec3 color;
#if defined(USE_MATERIAL)
	uint mapIndex = ubo_material.material[inMaterialID].mapIndex & 0xff;
	if (mapIndex == 0)
	{
		uint v = ubo_material.material[inMaterialID].albedoAlphaCompress;
		color = vec3((v & 0xff) / 255.0, ((v >> 8) & 0xff) / 255.0, ((v >> 16) & 0xff) / 255.0);
	}
	else
		color = texture(imgs_material[mapIndex - 1], inTexcoord).rgb;
#else
	color = pc.color.rgb;
#endif
	outColor = vec4(
		color
#if defined(USE_NORMAL)
		* dot(inNormal, vec3(0, 0, 1))
#endif
		,
#if defined(USE_MATERIAL)
		1.0
#else
		pc.color.a
#endif
	);
}