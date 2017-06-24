layout(binding = 0) uniform sampler2D tex;

layout(binding = 1) uniform COLOR
{
	vec4 v;
}u_color;

layout(location = 0) in vec2 inTexcoord;

layout(location = 0) out vec4 outColor;
		
void main()
{
	outColor = texture(tex, inTexcoord) * u_color.v;
}