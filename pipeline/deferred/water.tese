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

layout(quads, equal_spacing, ccw) in;

layout (location = 0) in flat uint inWaterId[];
layout (location = 1) in vec2 inUV[];
 
layout (location = 0) out flat uint outWaterId;
layout (location = 1) out vec2 outUV;

void main()
{
	vec2 uv0 = mix(inUV[0], inUV[1], gl_TessCoord.x);
	vec2 uv1 = mix(inUV[3], inUV[2], gl_TessCoord.x);
	outUV = mix(uv0, uv1, gl_TessCoord.y);

	vec4 pos0 = mix(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_TessCoord.x);
	vec4 pos1 = mix(gl_in[3].gl_Position, gl_in[2].gl_Position, gl_TessCoord.x);
	vec4 pos = mix(pos0, pos1, gl_TessCoord.y);
	//pos.y += texture(heightMap, outUV).r * u_water.d[inWaterId[0]].height;
	pos.xyz += u_water.d[inWaterId[0]].coord;
	gl_Position = u_matrix.projView * pos;
}