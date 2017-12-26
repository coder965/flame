#version 450 core
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

struct Terrain
{
	vec3 coord;
	int block_count;
	float block_size;
	float terrain_height;
	float displacement_height;
	float tessellation_factor;
	float tiling_scale;
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
	uint block_count = u_terrain.d[outTerrainId].block_count;
	float block_size = u_terrain.d[outTerrainId].block_size;
	float height = u_terrain.d[outTerrainId].terrain_height * inNormalHeight.a;
	vec3 coord = u_terrain.d[outTerrainId].coord;
	outUV = vec2((tileIndex % block_count) + (gl_VertexIndex & 2), (tileIndex / block_count) + ((gl_VertexIndex + 3) & 2));
	gl_Position = vec4(vec3((outUV.x - 0.5) * block_size, height, (outUV.y - 0.5) * block_size) + coord, 1.0);
	outUV /= block_count;
	mat3 normalMatrix = transpose(inverse(mat3(u_matrix.view)));
	outNormal = normalMatrix * inNormalHeight.rgb;
	outTangent = normalMatrix * inTangent;
}
