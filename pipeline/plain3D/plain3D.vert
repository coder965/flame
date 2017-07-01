layout(push_constant) uniform PushConstant
{
	mat4 modelview;
	mat4 proj;
	vec4 color;
}pc;

layout(location = 0) in vec3 inVertex;
#if defined(USE_NORMAL)
layout(location = 2) in vec3 inNormal;
#endif
		
void main()
{
	gl_Position = pc.proj * pc.modelview * vec4(inVertex, 1);
}