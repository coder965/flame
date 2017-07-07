layout(binding = 0) uniform MATRIX
{
	vec4 frustumPlanes[6];
	vec2 viewportDim;
}u_matrix;

layout(binding = 1) uniform TERRAIN
{
	mat4 projMatrix;
	mat4 viewMatrix;
	mat4 modelMatrix;
	uint patchSize;
	float ext;
	float height;
	float tessFactor;
	float mapDim;
}u_terrain;

layout (binding = 2) uniform sampler2D displacementMap;

layout (location = 0) in vec2 inUV;

layout(location = 0) out vec4 outAlbedoSpec;
layout(location = 1) out vec4 outNormalRoughness;

float getHeight(vec2 UV)
{
	return -texture(displacementMap, UV).r * u_terrain.height;
}

void main()
{
	mat3 normalMatrix = mat3(u_matrix.view);
	
	vec2 step = vec2(1.0 / u_terrain.mapDim, 0);
	float eps = (u_terrain.patchSize * u_terrain.ext) * step.x;
	
	float L  = getHeight(inUV - step.xy);
	float R  = getHeight(inUV + step.xy);
	float T  = getHeight(inUV - step.yx);
	float B  = getHeight(inUV + step.yx);
	
	vec3 normal = normalMatrix * normalize(vec3(L - R, 2.0 * eps, T - B));
	
	outAlbedoSpec = vec4(vec3(1.0), 0.05);
	outNormalRoughness = vec4(normal * 0.5 + 0.5, 1.0);
}
