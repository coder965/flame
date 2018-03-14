#if defined(USE_UV)
layout(location = 0) out vec2 outTexcoord;
#endif

#if defined(USE_VIEW)
layout(binding = 0) uniform ubo_constant_
{
	float near;
	float far;
	float cx;
	float cy;
	float aspect;
	float fovy;
	float tanHfFovy;
	float envrCx;
	float envrCy;
}ubo_constant;

layout(location = 
#if defined(USE_UV)
	1
#else
	0
#endif
) out vec3 outViewDir;
#endif

void main()
{
	vec2 v = vec2(gl_VertexIndex & 2, (gl_VertexIndex << 1) & 2);
#if defined(USE_UV)
	outTexcoord = v;
#endif
	v = v * 2.0 - 1.0;
	gl_Position = vec4(v, 0, 1);
#if defined(USE_VIEW)
	outViewDir = vec3(v * vec2(ubo_constant.tanHfFovy * ubo_constant.aspect, -ubo_constant.tanHfFovy), -1.0);
#endif
}