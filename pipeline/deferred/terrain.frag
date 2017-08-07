layout(binding = TKE_UBO_BINDING) uniform TERRAIN
{
	vec3 coord;
	int blockCx;
	float blockSize;
	float height;
	float tessellationFactor;
	float textureUvFactor;
	float mapDimension;
}u_terrain;

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

layout(binding = TKE_UBO_BINDING) uniform sampler2D heightMap;
layout(binding = TKE_UBO_BINDING) uniform sampler2D blendMap;
layout(binding = TKE_UBO_BINDING) uniform sampler2D colorMaps[4];
layout(binding = TKE_UBO_BINDING) uniform sampler2D normalMaps[4];

layout (location = 0) in vec2 inUV;

layout(location = 0) out vec4 outAlbedoAlpha;
layout(location = 1) out vec4 outNormalHeight;
layout(location = 2) out vec4 outSpecRoughness;

float getHeight(vec2 UV)
{
	return texture(heightMap, UV).r * u_terrain.height;
}

void main()
{
	mat3 normalMatrix = mat3(u_matrix.view);
	
	vec2 step = vec2(1.0 / u_terrain.mapDimension, 0);
	float eps = (u_terrain.blockCx * u_terrain.blockSize) * step.x * 2.0;

	float h = texture(heightMap, inUV).r;
	vec4 blend;
	blend.rgb = texture(blendMap, inUV).rgb;
	blend.a = 1.0 - blend.x - blend.y - blend.z;
	vec2 tilledUV = inUV * u_terrain.blockCx * u_terrain.textureUvFactor;

	vec3 color = vec3(0);
	color += texture(colorMaps[0], tilledUV).rgb * blend.r;
	color += texture(colorMaps[1], tilledUV).rgb * blend.g;
	color += texture(colorMaps[2], tilledUV).rgb * blend.b;
	color += texture(colorMaps[3], tilledUV).rgb * blend.a;
	vec3 normal = vec3(0);
	normal += (texture(normalMaps[0], tilledUV).xyz * 2.0 - 1.0) * blend.r;
	normal += (texture(normalMaps[1], tilledUV).xyz * 2.0 - 1.0) * blend.g;
	normal += (texture(normalMaps[2], tilledUV).xyz * 2.0 - 1.0) * blend.b;
	normal += (texture(normalMaps[3], tilledUV).xyz * 2.0 - 1.0) * blend.a;

	float LR  = getHeight(inUV - step.xy) - getHeight(inUV + step.xy);
	float TB  = getHeight(inUV - step.yx) - getHeight(inUV + step.yx);
	
	normal = normalMatrix * mat3(normalize(vec3(eps, 0.0, LR)), 
						normalize(vec3(0.0, TB, eps)), 
						normalize(vec3(LR, eps, TB))) * normal;
	
	outAlbedoAlpha = vec4(color, 1.0);
	outNormalHeight = vec4(normal * 0.5 + 0.5, 0.0);
	outSpecRoughness = vec4(0.05, 1.0, 0.0, 0.0);
}
