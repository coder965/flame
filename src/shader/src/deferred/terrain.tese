#version 450 core
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(binding = 1) uniform ubo_matrix_
{
	mat4 proj;
	mat4 projInv;
	mat4 view;
	mat4 viewInv;
	mat4 projView;
	mat4 projViewRotate;
	vec4 frustumPlanes[6];
	vec2 viewportDim;
}ubo_matrix;

layout(quads, equal_spacing, ccw) in;

layout (location = 0) in flat uint inTerrainId[];
layout (location = 1) in vec2 inUV[];
layout (location = 2) in vec3 inNormal[];
layout (location = 3) in vec3 inTangent[];
 
layout (location = 0) out flat uint outTerrainId;
layout (location = 1) out vec2 outUV;
layout (location = 2) out vec3 outNormal;
layout (location = 3) out vec3 outTangent;

void main()
{
	outTerrainId = inTerrainId[0];

	outUV = 
		mix(
			mix(inUV[0], inUV[1], gl_TessCoord.x), 
			mix(inUV[3], inUV[2], gl_TessCoord.x), 
			gl_TessCoord.y
		);

	outNormal = 
		mix(
			mix(inNormal[0], inNormal[1], gl_TessCoord.x), 
			mix(inNormal[3], inNormal[2], gl_TessCoord.x), 
			gl_TessCoord.y
		);

	outTangent = 
		mix(
			mix(inTangent[0], inTangent[1], gl_TessCoord.x), 
			mix(inTangent[3], inTangent[2], gl_TessCoord.x), 
			gl_TessCoord.y
		);

	vec4 pos = 
		mix(
			mix(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_TessCoord.x), 
			mix(gl_in[3].gl_Position, gl_in[2].gl_Position, gl_TessCoord.x), 
			gl_TessCoord.y
		);
	gl_Position = ubo_matrix.projView * pos;
}