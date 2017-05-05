struct Terrain
{
	uint patchSize;
	float ext;
	float height;
	float tessFactor;
	float mapDim;
};

layout(binding = 1) uniform TERRAIN
{
	Terrain data[8];
}u_terrain;

layout (location = 0) out flat uint outIndex;
layout (location = 1) out vec2 outUV;

void main(void)
{
	outIndex = gl_InstanceIndex / 0xffff;
	
	uint patchSize = u_terrain.data[outIndex].patchSize;
	
	uint tileIndex = gl_InstanceIndex & 0xffff;
	outUV = vec2((tileIndex % patchSize) + (gl_VertexIndex & 2), (tileIndex / patchSize) + ((gl_VertexIndex + 3) & 2));
	gl_Position = vec4(outUV.x * u_terrain.data[outIndex].ext, 0.0, outUV.y * u_terrain.data[outIndex].ext, 1.0);
	outUV /= patchSize;
}