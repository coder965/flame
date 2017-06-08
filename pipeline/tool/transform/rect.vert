layout(binding = 0) uniform RECT_DATA
{
	vec4 data; // pos + size
}u_data;

void main()
{
	vec2 pos = vec2((gl_VertexIndex & 2) - 1.0, ((gl_VertexIndex + 1) & 2) - 1.0) * u_data.data.zw + u_data.data.xy;
	gl_Position = vec4(pos, 0.0, 1.0);
}