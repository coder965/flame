#version 450 core
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(binding = 0) uniform sampler2D sdf_image;

layout(location = 0) in vec2 inUV;

layout(location = 0) out vec4 fColor;

float median(float r, float g, float b) {
    return max(min(r, g), min(max(r, g), b));
}
 
void main()
{
	vec2 uv = inUV;
	uv.y = 1 - uv.y;
    vec2 msdfUnit = vec2(4) / vec2(32 * 63, 32);
    vec3 sam = texture(sdf_image, uv).rgb;
    float sigDist = median(sam.r, sam.g, sam.b) - 0.5;
    sigDist *= dot(msdfUnit, 0.5/fwidth(uv));
    float opacity = clamp(sigDist + 0.5, 0.0, 1.0);
	fColor = vec4(0, 0, 0, opacity);
}
