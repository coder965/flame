layout(location = 0) in flat uint inInstanceIndex;

layout(location = 0) out vec4 outColor;
		
void main()
{
	outColor = vec4((inInstanceIndex + 1) / 255.0, 0.0, 0.0, 0.0);
}