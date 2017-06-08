layout(location = 0) out vec2 outTexcoord;

void main()
{
	outTexcoord = vec2(gl_VertexIndex & 2, (gl_VertexIndex << 1) & 2);
	gl_Position = vec4(outTexcoord * 2.0 - 1.0, 0, 1);
}