#version 450 core
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(binding = 1) uniform TERRAIN
{
	vec2 seed;
}u_terrain;

layout (location = 0) out vec2 outUV;

void main(void)
{
	vec2 seed = u_terrain.seed;
	
	uint patchSize = 100;
	
	uint tileIndex = gl_InstanceIndex & 0xffff;
	outUV = vec2((tileIndex % patchSize) + (gl_VertexIndex & 2), (tileIndex / patchSize) + ((gl_VertexIndex + 3) & 2));
	gl_Position = vec4(outUV.x * 10.0, 0.0, outUV.y * 10.0, 1.0)
		+ vec4(seed.x * 1000.0, 0.0, seed.y * 1000.0, 0.0);
	outUV /= patchSize;
}
