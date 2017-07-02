layout(push_constant) uniform PushConstant
{
	mat4 modelview;
	mat4 proj;
	vec4 color;
}pc;

#if defined(USE_NORMAL)
layout(location = 0) in vec3 inNormal;
#endif

layout(location = 0) out vec4 outColor;
		
void main()
{
	outColor = vec4(pc.color.rgb
#if defined(USE_NORMAL)
	* dot(inNormal, vec3(0, 0, -1))
#endif
	, pc.color.a);
}