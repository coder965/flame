#version 450 core
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

struct Terrain
{
	vec3 coord;
	int blockCx;
	float blockSize;
	float height;
	float tessellationFactor;
	float textureUvFactor;
	float mapDimension;
};

layout(binding = 4) uniform TERRAIN
{
	Terrain d[8];
}u_terrain;

layout (location = 0) out flat uint outTerrainId;
layout (location = 1) out vec2 outUV;

void main(void)
{
	outTerrainId = gl_InstanceIndex >> 16;
	uint tileIndex = gl_InstanceIndex & 0xffff;
	outUV = vec2((tileIndex % u_terrain.d[outTerrainId].blockCx) + (gl_VertexIndex & 2), (tileIndex / u_terrain.d[outTerrainId].blockCx) + ((gl_VertexIndex + 3) & 2));
	gl_Position = vec4(outUV.x * u_terrain.d[outTerrainId].blockSize, 0.0, outUV.y * u_terrain.d[outTerrainId].blockSize, 1.0);
	outUV /= vec2(u_terrain.d[outTerrainId].blockCx, u_terrain.d[outTerrainId].blockCx);
}
