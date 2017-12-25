#version 450 core
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

struct Water
{
	vec3 coord;
	int blockCx;
	float blockSize;
	float height;
	float tessellationFactor;
	float textureUvFactor;
	float mapDimension;
};

layout(binding = 5) uniform WATER
{
	Water d[8];
}u_water;

layout (location = 0) out flat uint outWaterId;
layout (location = 1) out vec2 outUV;

void main(void)
{
	outWaterId = gl_InstanceIndex >> 16;
	uint tileIndex = gl_InstanceIndex & 0xffff;
	outUV = vec2((tileIndex % u_water.d[outWaterId].blockCx) + (gl_VertexIndex & 2), (tileIndex / u_water.d[outWaterId].blockCx) + ((gl_VertexIndex + 3) & 2));
	gl_Position = vec4(outUV.x * u_water.d[outWaterId].blockSize, 0.0, outUV.y * u_water.d[outWaterId].blockSize, 1.0);
	outUV /= vec2(u_water.d[outWaterId].blockCx, u_water.d[outWaterId].blockCx);
}
