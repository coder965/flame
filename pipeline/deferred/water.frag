struct WATER
{
	vec3 coord;
	int blockCx;
	float blockSize;
	float height;
	float tessellationFactor;
	float textureUvFactor;
	float mapDimension;
};

layout(binding = TKE_UBO_BINDING) uniform WATER
{
	Water d[8];
}u_water;

layout(binding = TKE_UBO_BINDING) uniform MATRIX
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

layout (location = 0) in flat uint inWaterId;
layout (location = 1) in vec2 inUV;

layout(location = 0) out vec4 outAlbedoAlpha;
layout(location = 1) out vec4 outNormalHeight;
layout(location = 2) out vec4 outSpecRoughness;

void main()
{
	mat3 normalMatrix = mat3(u_matrix.view);
	
	//vec3 normal = normalMatrix * normalize(vec3(L - R, 2.0 * eps, T - B));
	vec3 normal = vec3(0, 1, 0);
	
	outAlbedoAlpha = vec4(1.0, 1.0, 1.0, 1.0);
	outNormalHeight = vec4(normal * 0.5 + 0.5, 0.0);
	outSpecRoughness = vec4(0.05, 1.0, 0.0, 0.0);
}
