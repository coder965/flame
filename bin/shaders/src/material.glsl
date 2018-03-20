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
