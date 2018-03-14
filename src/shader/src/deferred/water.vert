struct Water
{
	vec3 coord;
	int block_cx;
	int block_cy;
	float block_size;
	float height;
	float tessellation_factor;
	float tiling_scale;
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
	outUV = vec2((tileIndex % ubo_water.d[outWaterId].block_cx) + (gl_VertexIndex & 2), (tileIndex / ubo_water.d[outWaterId].block_cx) + ((gl_VertexIndex + 3) & 2));
	gl_Position = vec4(outUV.x * ubo_water.d[outWaterId].block_size, 0.0, outUV.y * ubo_water.d[outWaterId].block_size, 1.0);
	outUV /= vec2(ubo_water.d[outWaterId].block_cx, ubo_water.d[outWaterId].block_cy);
}
