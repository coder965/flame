struct Light
{
	vec4 coord;
	vec4 color;
	vec4 spotData;
};

layout(binding = 10) uniform ubo_light_
{
	uint count;
	Light lights[256];
}ubo_light;
