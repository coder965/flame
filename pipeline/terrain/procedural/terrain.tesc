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

layout (vertices = 4) out;
 
layout (location = 0) in vec2 inUV[];
 
layout (location = 0) out vec2 outUV[4];
 
float screenSpaceTessFactor(vec4 p0, vec4 p1)
{
	vec4 midPoint = 0.5 * (p0 + p1);
	float radius = distance(p0, p1) / 2.0;

	vec4 v0 = u_matrix.matrixView * midPoint;

	vec4 clip0 = (u_matrix.matrixProj * (v0 - vec4(radius, vec3(0.0))));
	vec4 clip1 = (u_matrix.matrixProj * (v0 + vec4(radius, vec3(0.0))));

	clip0 /= clip0.w;
	clip1 /= clip1.w;

	clip0.xy *= u_matrix.viewportDim;
	clip1.xy *= u_matrix.viewportDim;
	
	return clamp(distance(clip0, clip1) / 10.0 * 0.75, 1.0, 64.0);
}

bool frustumCheck()
{
	vec2 uv = (inUV[0] + inUV[1] + inUV[2] + inUV[3]) * 0.25;
	
	const float radius = max(10.0, 100.0);
	vec4 pos = (gl_in[0].gl_Position + gl_in[1].gl_Position + gl_in[2].gl_Position + gl_in[3].gl_Position) * 0.25;
	pos.y -= fbm2d((uv + u_terrain.seed.xy) * 1000.0) * 300.0;
	pos = u_matrix.matrixProjView * pos;
	pos = pos / pos.w;

	for (int i = 0; i < 6; i++) 
	{
		if (dot(pos, u_matrix.frustumPlanes[i]) + radius < 0.0)
			return false;
	}
	return true;
}

void main()
{
	if (gl_InvocationID == 0)
	{
		if (!frustumCheck())
		{
			gl_TessLevelInner[0] = 0.0;
			gl_TessLevelInner[1] = 0.0;
			gl_TessLevelOuter[0] = 0.0;
			gl_TessLevelOuter[1] = 0.0;
			gl_TessLevelOuter[2] = 0.0;
			gl_TessLevelOuter[3] = 0.0;
		}
		else
		{
			
			if (0.75 > 0.0)
			{
				gl_TessLevelOuter[0] = screenSpaceTessFactor(gl_in[3].gl_Position, gl_in[0].gl_Position);
				gl_TessLevelOuter[1] = screenSpaceTessFactor(gl_in[0].gl_Position, gl_in[1].gl_Position);
				gl_TessLevelOuter[2] = screenSpaceTessFactor(gl_in[1].gl_Position, gl_in[2].gl_Position);
				gl_TessLevelOuter[3] = screenSpaceTessFactor(gl_in[2].gl_Position, gl_in[3].gl_Position);
				gl_TessLevelInner[0] = mix(gl_TessLevelOuter[0], gl_TessLevelOuter[3], 0.5);
				gl_TessLevelInner[1] = mix(gl_TessLevelOuter[2], gl_TessLevelOuter[1], 0.5);
			}
			else
			{
				gl_TessLevelInner[0] = 1.0;
				gl_TessLevelInner[1] = 1.0;
				gl_TessLevelOuter[0] = 1.0;
				gl_TessLevelOuter[1] = 1.0;
				gl_TessLevelOuter[2] = 1.0;
				gl_TessLevelOuter[3] = 1.0;
			}
		}
	}

	gl_out[gl_InvocationID].gl_Position =  gl_in[gl_InvocationID].gl_Position;
	outUV[gl_InvocationID] = inUV[gl_InvocationID];
} 