#include "..\..\noise.h"

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

layout(binding = 1) uniform TERRAIN
{
	vec2 seed;
}u_terrain;

layout (location = 0) in vec2 inUV;

layout(location = 0) out vec4 outAlbedoSpec;
layout(location = 1) out vec4 outNormalRoughness;

void main()
{
	mat3 normalMatrix = mat3(u_matrix.view);
	
	vec2 eps = vec2(0.02, 0.0);
	vec2 uv = (inUV + vec2(u_terrain.seed.xy)) * 1000.0;
	
	float L  = fbm2d(uv - eps.xy) * 300.0;
	float R  = fbm2d(uv + eps.xy) * 300.0;
	float T  = fbm2d(uv - eps.yx) * 300.0;
	float B  = fbm2d(uv + eps.yx) * 300.0;
	
	vec3 normal = normalMatrix * normalize(vec3(R - L, 2.0 * eps.x, B - T));
	//vec3 normal = normalize(vec3(B - T));
	
	outAlbedoSpec = vec4(vec3(1.0), 0.05);
	outNormalRoughness = vec4(normal * 0.5 + 0.5, 1.0);
}
