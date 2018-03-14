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

layout (location = 0) in vec4 inNormalHeight;
layout (location = 1) in vec3 inTangent;

layout (location = 0) out flat uint outTerrainId;
layout (location = 1) out vec2 outUV;
layout (location = 2) out vec3 outNormal;
layout (location = 3) out vec3 outTangent;

void main(void)
{
	outTerrainId = gl_InstanceIndex >> 16;
	uint tileIndex = gl_InstanceIndex & 0xffff;
	ivec2 block_count = ivec2(ubo_terrain.d[outTerrainId].block_cx, ubo_terrain.d[outTerrainId].block_cy);
	float block_size = ubo_terrain.d[outTerrainId].block_size;
	float height = ubo_terrain.d[outTerrainId].terrain_height * inNormalHeight.a;
	vec3 coord = ubo_terrain.d[outTerrainId].coord;
	outUV = vec2((tileIndex % block_count.x) + (gl_VertexIndex & 2), (tileIndex / block_count.x) + ((gl_VertexIndex + 3) & 2));
	gl_Position = vec4(vec3((outUV.x - 0.5) * block_size, height, (outUV.y - 0.5) * block_size) + coord, 1.0);
	outUV /= block_count;
	mat3 normalMatrix = transpose(inverse(mat3(ubo_matrix.view)));
	outNormal = normalMatrix * inNormalHeight.rgb;
	outTangent = normalMatrix * inTangent;
}
