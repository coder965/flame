#version 450 core
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(binding = 0) uniform sampler2D sdf_image;

layout(location = 0) in vec2 inUV;

layout(location = 0) out vec4 fColor;
 
void main()
{
	float d = texture(sdf_image, inUV).r;
	if (d < 0.999)
		discard;
	fColor = vec4(0, 0, 0, 1);
}
