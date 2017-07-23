struct Material
{
	uint albedoAlphaCompress;
	uint specRoughnessCompress;

	uint mapIndex;
	
	uint dummy;
};

layout(binding = TKE_UBO_BINDING) uniform MATERIAL
{
	Material material[256];
}u_material;

layout(set = 1, binding = TKE_UBO_BINDING) uniform sampler2D maps[256];

layout(location = 0) in vec2 inTexcoord;
		
void main()
{
	uint mapIndex;

	vec3 albedo;
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
	
	outAlbedoAlpha = vec4(albedo, alpha);
}