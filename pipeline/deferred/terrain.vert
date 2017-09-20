layout(binding = 4) uniform TERRAIN
{
	vec3 coord;
	int blockCx;
	float blockSize;
	float height;
	float tessellationFactor;
	float textureUvFactor;
	float mapDimension;
}u_terrain;

layout (location = 0) out vec2 outUV;

void main(void)
{
	uint tileIndex = gl_InstanceIndex & 0xffff;
	outUV = vec2((tileIndex % u_terrain.blockCx) + (gl_VertexIndex & 2), (tileIndex / u_terrain.blockCx) + ((gl_VertexIndex + 3) & 2));
	gl_Position = vec4(outUV.x * u_terrain.blockSize, 0.0, outUV.y * u_terrain.blockSize, 1.0);
	outUV /= vec2(u_terrain.blockCx, u_terrain.blockCx);
}