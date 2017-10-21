#version 450 core
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(binding = 0) uniform CONSTANT
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
}u_constant;

layout(location = 0) out vec2 outTexcoord;
layout(location = 1) out vec3 outViewDir;

void main()
{
	outTexcoord = vec2(gl_VertexIndex & 2, (gl_VertexIndex << 1) & 2);
	vec2 c = outTexcoord * 2.0 - 1.0;
	gl_Position = vec4(c, 0, 1);
	outViewDir = vec3(c * vec2(u_constant.tanHfFovy * u_constant.aspect, -u_constant.tanHfFovy), -1.0);
}
