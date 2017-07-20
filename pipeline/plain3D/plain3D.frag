layout(push_constant) uniform PushConstant
{
	mat4 modelview;
	mat4 proj;
#if !defined(USE_TEX)
	vec4 color;
#endif
}pc;

#if defined(USE_TEX)
layout(binding = TKE_UBO_BINDING) uniform sampler2D tex;
#endif

#if defined(USE_TEX)
layout(location = 0) in vec2 inTexcoord;
#endif
#if defined(USE_NORMAL)
layout(location = 1) in vec3 inNormal;
#endif

layout(location = 0) out vec4 outColor;
		
void main()
{
	outColor = vec4(
#if defined(USE_TEX)
		texture(tex, inTexcoord).rgb
#else
		pc.color.rgb
#endif
#if defined(USE_NORMAL)
	* dot(inNormal, vec3(0, 0, 1))
#endif
	, pc.color.a);
}