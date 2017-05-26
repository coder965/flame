#version 450 core
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(binding = 0) uniform MATRIX
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

struct Terrain
{
	uint patchSize;
	float ext;
	float height;
	float tessFactor;
	float mapDim;
};

layout(binding = 1) uniform TERRAIN
{
	Terrain data[8];
}u_terrain;

layout (binding = 2) uniform sampler2D displacementMap[8]; 

layout (location = 0) in flat uint inIndex;
layout (location = 1) in vec2 inUV;

layout(location = 0) out vec4 outAlbedoSpec;
layout(location = 1) out vec4 outNormalRoughness;

float getHeight(vec2 UV)
{
	return -texture(displacementMap[inIndex], UV).r * u_terrain.data[inIndex].height;
}

void main()
{
	mat3 normalMatrix = mat3(u_matrix.view);
	
	vec2 step = vec2(1.0 / u_terrain.data[inIndex].mapDim, 0);
	float eps = (u_terrain.data[inIndex].patchSize * u_terrain.data[inIndex].ext) * step.x;
	
	float L  = getHeight(inUV - step.xy);
	float R  = getHeight(inUV + step.xy);
	float T  = getHeight(inUV - step.yx);
	float B  = getHeight(inUV + step.yx);
	
	vec3 normal = normalMatrix * normalize(vec3(L - R, 2.0 * eps, T - B));
	
	outAlbedoSpec = vec4(vec3(1.0), 0.05);
	outNormalRoughness = vec4(normal * 0.5 + 0.5, 1.0);
}
