layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aUV;

layout(location = 0) out vec2 outUV;

void main()
{
	outUV = aUV;
	gl_Position = vec4(aPos, 0, 1);
}