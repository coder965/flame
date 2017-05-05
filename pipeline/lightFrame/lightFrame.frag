layout(location = 0) in vec3 inNormal;

layout(location = 0) out vec4 outColor;
		
void main()
{
	outColor = vec4(vec3(dot(inNormal, vec3(0.0, 0.0, 1.0))), 1.0);
}