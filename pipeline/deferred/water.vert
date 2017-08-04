layout(binding = TKE_UBO_BINDING) uniform WATER
{
	int blockCx;
	float blockSize;
	float height;
	float tessellationFactor;
	float textureUvFactor;
	float mapDimension;
}u_water[8];

layout (location = 0) out flat uint outWaterId;
layout (location = 1) out vec2 outUV;

void main(void)
{
	outWaterId = gl_InstanceIndex >> 16;
	uint tileIndex = gl_InstanceIndex & 0xffff;
	outUV = vec2((tileIndex % u_water[outWaterId].blockCx) + (gl_VertexIndex & 2), (tileIndex / u_water[outWaterId].blockCx) + ((gl_VertexIndex + 3) & 2));
	gl_Position = vec4(outUV.x * u_water[outWaterId].blockSize, 0.0, outUV.y * u_water[outWaterId].blockSize, 1.0);
	outUV /= vec2(u_water[outWaterId].blockCx, u_water[outWaterId].blockCx);
}