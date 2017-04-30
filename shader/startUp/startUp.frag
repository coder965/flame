layout(binding = 0) uniform sampler2D tex;
layout(binding = 1) uniform PROGRESS
{
	vec2 value;
}u_progress;

layout(location = 0) in vec2 inTexcoord;

layout(location = 0) out vec4 outColor;
		
void main()
{
	vec3 color = texture(tex, inTexcoord).rgb;
	float v;
	if (inTexcoord.y < 0.5)
		v = u_progress.value.r;
	else
		v = u_progress.value.g;
	if (inTexcoord.x > v)
		outColor = vec4(color, 1);
	else
		outColor = vec4(1 - color, 1);
}