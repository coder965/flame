layout(location = 0) in vec2 inTexcoord;
layout(location = 1) in vec3 inNormal;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D img_tex;
		
void main()
{
	outColor = vec4(texture(img_tex, inTexcoord).rgb * abs(dot(inNormal, vec3(0, 0, 1))), 1.0);
}
