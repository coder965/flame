layout(push_constant) uniform PushConstant
{
	mat4 mvp;
}pc;

layout(location = 0) in vec3 inVertex;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 outColor;

void main()
{
	outColor = inColor;
	gl_Position = pc.mvp * vec4(inVertex, 1);
}