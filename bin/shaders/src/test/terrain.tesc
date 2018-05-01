#include "terrain.glsl"

layout (vertices = 4) out;
 
layout (location = 0) in vec2 inUV[];
 
layout (location = 0) out vec2 outUV[4];

float screenSpaceTessFactor(vec4 p0, vec4 p1)
{
	float radius = distance(p0, p1) / 2.0;

	vec4 v0 = ubo_terrain.view_matrix * (0.5 * (p0 + p1));

	vec4 clip0 = (ubo_terrain.proj_matrix * (v0 - vec4(radius, vec3(0.0))));
	vec4 clip1 = (ubo_terrain.proj_matrix * (v0 + vec4(radius, vec3(0.0))));

	clip0 /= clip0.w;
	clip1 /= clip1.w;

	clip0.xy *= ubo_terrain.resolution;
	clip1.xy *= ubo_terrain.resolution;
	
	return clamp(distance(clip0, clip1) / ubo_terrain.size * ubo_terrain.tessellation_factor, 1.0, 64.0);
}

void main()
{
	outUV[gl_InvocationID] = inUV[gl_InvocationID];

	if (gl_InvocationID == 0)
	{
		gl_TessLevelOuter[0] = screenSpaceTessFactor(gl_in[3].gl_Position, gl_in[0].gl_Position);
		gl_TessLevelOuter[1] = screenSpaceTessFactor(gl_in[0].gl_Position, gl_in[1].gl_Position);
		gl_TessLevelOuter[2] = screenSpaceTessFactor(gl_in[1].gl_Position, gl_in[2].gl_Position);
		gl_TessLevelOuter[3] = screenSpaceTessFactor(gl_in[2].gl_Position, gl_in[3].gl_Position);
		gl_TessLevelOuter[0] = 4;
		gl_TessLevelOuter[1] = 4;
		gl_TessLevelOuter[2] = 4;
		gl_TessLevelOuter[3] = 4;
		gl_TessLevelInner[0] = mix(gl_TessLevelOuter[0], gl_TessLevelOuter[3], 0.5);
		gl_TessLevelInner[1] = mix(gl_TessLevelOuter[2], gl_TessLevelOuter[1], 0.5);
	}

	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
} 
