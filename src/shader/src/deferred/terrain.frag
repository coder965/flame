layout(binding = 1) uniform ubo_matrix_
{
	mat4 proj;
	mat4 projInv;
	mat4 view;
	mat4 viewInv;
	mat4 projView;
	mat4 projViewRotate;
	vec4 frustumPlanes[6];
	vec2 viewportDim;
}ubo_matrix;

struct Terrain
{
	vec3 coord;
	int block_cx;
	int block_cy;
	float block_size;
	float terrain_height;
	float displacement_height;
	float tessellation_factor;
	float tiling_scale;
	uint material_count;
	uint material_index;
};

layout(binding = 3) uniform ubo_terrain_
{
	Terrain d[8];
}ubo_terrain;

layout(binding = 4) uniform sampler2D imgs_blend[8];

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

layout (location = 0) in flat uint inTerrainId;
layout (location = 1) in vec2 inUV;
layout (location = 2) in vec3 inNormal;
layout (location = 3) in vec3 inTangent;

layout(location = 0) out vec4 outAlbedoAlpha;
layout(location = 1) out vec4 outNormalHeight;
layout(location = 2) out vec4 outSpecRoughness;

vec2 tilledUV;
vec3 albedo;
vec3 normal;
float spec, roughness;

void get_material(uint mat_index, float blend)
{
	uint mapIndex;

	mapIndex = ubo_material.material[mat_index].mapIndex & 0xff;
	if (mapIndex == 0)
	{
		uint v = ubo_material.material[mat_index].albedoAlphaCompress;
		albedo += vec3((v & 0xff) / 255.0, ((v >> 8) & 0xff) / 255.0, ((v >> 16) & 0xff) / 255.0) * blend;
	}
	else
	{
		vec4 v = texture(imgs_material[mapIndex - 1], tilledUV);
		albedo += v.rgb * blend;
	}
	
	mapIndex = (ubo_material.material[mat_index].mapIndex >> 8) & 0xff;
	if (mapIndex > 0)
	{
		vec4 v = texture(imgs_material[mapIndex - 1], tilledUV);
		vec3 tn = normalize(v.xyz * 2.0 - 1.0);
		normal += normalize(mat3(-inTangent, cross(normal, -inTangent), normal) * tn) * blend;
	}
	else
		normal += inNormal * blend;

	mapIndex = (ubo_material.material[mat_index].mapIndex >> 16) & 0xff;
	if (mapIndex == 0)
	{
		uint v = ubo_material.material[mat_index].specRoughnessCompress;
		spec += (v & 0xff) / 255.0 * blend;
		roughness += ((v >> 8) & 0xff) / 255.0 * blend;
	}
	else
	{
		vec4 v = texture(imgs_material[mapIndex - 1], tilledUV);
		spec += v.r * blend;
		roughness += v.g * blend;
	}
}

void main()
{
	albedo = vec3(0.0);
	normal = vec3(0.0);
	spec = 0.0;
	roughness = 0.0;

	vec4 blend = texture(imgs_blend[inTerrainId], inUV);
	tilledUV = inUV * ubo_terrain.d[inTerrainId].tiling_scale * 
		vec2(ubo_terrain.d[inTerrainId].block_cx, ubo_terrain.d[inTerrainId].block_cy);

	uint curr_mat_index = ubo_terrain.d[inTerrainId].material_index;
	for(int i = 0 ; i < ubo_terrain.d[inTerrainId].material_count; i++)
	{
		get_material(curr_mat_index & 0xff, blend[i]);
		curr_mat_index >> 8;
	}

	outAlbedoAlpha = vec4(albedo, 1.0);
	outNormalHeight = vec4(normal * 0.5 + 0.5, 0.0);
	outSpecRoughness = vec4(spec, roughness, 0.0, 0.0);
}
