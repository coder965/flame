layout(location = 0) in flat uint inIndex;

layout(location = 0) out vec4 outIndex;
		
void main()
{
	uint index = inIndex + 1;
	outIndex = vec4((index & 0xff) / 255.0, ((index >> 8) & 0xff) / 255.0, ((index >> 16) & 0xff) / 255.0, ((index >> 24) & 0xff) / 255.0);
}