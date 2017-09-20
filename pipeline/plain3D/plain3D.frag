#if !defined(USE_TEX)
layout(push_constant) uniform PushConstant
{
	mat4 modelview;
	mat4 proj;
	vec4 color;
}pc;
#endif

#if defined(USE_TEX)
layout(set = 1, binding = 0) uniform sampler2D maps[256];
#endif

#if defined(USE_TEX)
layout(location = 0) in vec2 inTexcoord;
layout(location = 3) in flat uint inTexId;
#endif
#if defined(USE_NORMAL)
layout(location = 1) in vec3 inNormal;
#endif

layout(location = 0) out vec4 outColor;
		
void main()
{
	outColor = vec4(
#if defined(USE_TEX)
		texture(maps[inTexId], inTexcoord).rgb
#else
		pc.color.rgb
#endif
#if defined(USE_NORMAL)
		* dot(inNormal, vec3(0, 0, 1))
#endif
	,
#if defined(USE_TEX)
		1.0
#else
		pc.color.a
#endif
	);
}