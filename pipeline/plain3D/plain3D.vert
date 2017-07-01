layout(push_constant) uniform PushConstant
{
	mat4 matrix;
	vec4 color;
}pc;

layout(location = 0) in vec3 inVertex;
		
void main()
{
	gl_Position = pc.matrix * vec4(inVertex, 1);
}