#include "..\..\noise.h"

layout(binding = 0) uniform MATRIX
{
	mat4 matrixProj;
	mat4 matrixProjInv;
	mat4 matrixView;
	mat4 matrixViewInv;
	mat4 matrixProjView;
	mat4 matrixProjViewRotate;
	vec4 frustumPlanes[6];
	vec2 viewportDim;
}u_matrix;

layout(binding = 1) uniform TERRAIN
{
	vec2 seed;
}u_terrain;

layout(quads, equal_spacing, ccw) in;

layout (location = 0) in vec2 inUV[];
 
layout (location = 0) out vec2 outUV;

void main()
{
	vec2 uv0 = mix(inUV[0], inUV[1], gl_TessCoord.x);
	vec2 uv1 = mix(inUV[3], inUV[2], gl_TessCoord.x);
	outUV = mix(uv0, uv1, gl_TessCoord.y);

	vec4 pos0 = mix(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_TessCoord.x);
	vec4 pos1 = mix(gl_in[3].gl_Position, gl_in[2].gl_Position, gl_TessCoord.x);
	vec4 pos = mix(pos0, pos1, gl_TessCoord.y);
	pos.y -= fbm2d((outUV + u_terrain.seed.xy) * 1000.0) * 300.0;
	gl_Position = u_matrix.matrixProjView * pos;
}
