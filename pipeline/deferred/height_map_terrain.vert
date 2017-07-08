#include "terrain.h"

layout(binding = TKE_UBO_BINDING) uniform TERRAIN
{
	float ext;
	float height;
	float tessFactor;
	float mapDim;
}u_terrain;

layout (location = 0) out vec2 outUV;

void main(void)
{
	uint tileIndex = gl_InstanceIndex & 0xffff;
	outUV = vec2((tileIndex % PATCH_SIZE) + (gl_VertexIndex & 2), (tileIndex / PATCH_SIZE) + ((gl_VertexIndex + 3) & 2));
	gl_Position = vec4(outUV.x * u_terrain.ext, 0.0, outUV.y * u_terrain.ext, 1.0);
	outUV /= PATCH_SIZE;
}