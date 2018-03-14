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

struct Water
{
	vec3 coord;
	int block_cx;
	int block_cy;
	float block_size;
	float height;
	float tessellation_factor;
	float tiling_scale;
	float mapDimension;
};

layout(binding = 5) uniform ubo_water_
{
	Water d[8];
}ubo_water;

layout (location = 0) in flat uint inWaterId;
layout (location = 1) in vec2 inUV;

layout(location = 0) out vec4 outAlbedoAlpha;
layout(location = 1) out vec4 outNormalHeight;
layout(location = 2) out vec4 outSpecRoughness;

void main()
{
	mat3 normalMatrix = transpose(inverse(mat3(ubo_matrix.view)));
	
	//vec3 normal = normalMatrix * normalize(vec3(L - R, 2.0 * eps, T - B));
	vec3 normal = normalMatrix * vec3(0, 1, 0);
	
	outAlbedoAlpha = vec4(1.0, 1.0, 1.0, 1.0);
	outNormalHeight = vec4(normal * 0.5 + 0.5, 0.0);
	outSpecRoughness = vec4(1.0, 0.05, 0.0, 0.0);
}
