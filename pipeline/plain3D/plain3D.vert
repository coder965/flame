layout(push_constant) uniform PushConstant
{
	mat4 modelview;
	mat4 proj;
	vec4 color;
}pc;

layout(location = 0) in vec3 inVertex;
#if defined(USE_NORMAL)
layout(location = 2) in vec3 inNormal;
layout(location = 0) out vec3 outNormal;
#endif
		
void main()
{
#if defined(USE_NORMAL)
	mat3 normalMatrix = transpose(inverse(mat3(pc.modelview)));
	outNormal = normalize(normalMatrix * inNormal);
#endif
	gl_Position = pc.proj * pc.modelview * vec4(inVertex, 1);
}