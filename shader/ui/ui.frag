layout(binding = 0) uniform sampler2D sTexture[512];

layout(location = 0) in vec4 inColor;
layout(location = 1) in vec2 inUV;
layout(location = 2) in flat uint inID;

layout(location = 0) out vec4 fColor;

void main()
{
	fColor = inColor * texture(sTexture[inID], inUV);
}