layout(binding = 14) uniform ubo_shadow_
{
	mat4 matrix[8];
}ubo_shadow;

layout(binding = 15) uniform sampler2DArray img_shadow;
