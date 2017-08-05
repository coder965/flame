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
	float eps = (u_terrain.blockCx * u_terrain.blockSize) * step.x;
	
	float L  = getHeight(inUV - step.xy);
	float R  = getHeight(inUV + step.xy);
	float T  = getHeight(inUV - step.yx);
	float B  = getHeight(inUV + step.yx);
	
	vec3 normal = normalMatrix * normalize(vec3(L - R, 2.0 * eps, T - B));

	vec3 color = vec3(0);
	float h = texture(heightMap, inUV).r;
	vec2 tilledUV = inUV * u_terrain.blockCx * u_terrain.textureUvFactor;
	if (h < 0.33)
	{
		float v = h / 0.33;
		color = mix(texture(colorMaps[0], tilledUV).rgb, texture(colorMaps[1], tilledUV).rgb, v);
	}
	else if (h < 0.66)
	{
		float v = (h - 0.33) / 0.33;
		color = mix(texture(colorMaps[1], tilledUV).rgb, texture(colorMaps[2], tilledUV).rgb, v);
	}
	else
	{
		float v = (h - 0.66) / 0.33;
		color = mix(texture(colorMaps[2], tilledUV).rgb, texture(colorMaps[3], tilledUV).rgb, v);
	}
	
	outAlbedoAlpha = vec4(color, 1.0);
	outNormalHeight = vec4(normal * 0.5 + 0.5, 0.0);
	outSpecRoughness = vec4(0.05, 1.0, 0.0, 0.0);
}
