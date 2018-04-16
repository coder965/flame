layout(binding = 0) uniform ubo_test_
{
	vec4 color;
}ubo_test;

layout(location = 0) out vec4 outColor;

void main()
{
	outColor = ubo_test.color;
	//outColor = vec4(0.7, 0.6, 0.4, 1.0);
}
