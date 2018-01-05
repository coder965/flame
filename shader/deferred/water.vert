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

layout(binding = 5) uniform ubo_water_
{
	Water d[8];
}ubo_water;

layout (location = 0) out flat uint outWaterId;
layout (location = 1) out vec2 outUV;

void main(void)
{
	outWaterId = gl_InstanceIndex >> 16;
	uint tileIndex = gl_InstanceIndex & 0xffff;
	outUV = vec2((tileIndex % ubo_water.d[outWaterId].blockCx) + (gl_VertexIndex & 2), (tileIndex / ubo_water.d[outWaterId].blockCx) + ((gl_VertexIndex + 3) & 2));
	gl_Position = vec4(outUV.x * ubo_water.d[outWaterId].blockSize, 0.0, outUV.y * ubo_water.d[outWaterId].blockSize, 1.0);
	outUV /= vec2(ubo_water.d[outWaterId].blockCx, ubo_water.d[outWaterId].blockCx);
}
