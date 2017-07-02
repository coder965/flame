layout(binding = 0) uniform sampler2D originalSampler;

layout(location = 0) out vec4 outColor;

const float gamma = 0.45455;
//const float gamma = 1.0;

void main()
{
	outColor = vec4(pow(texture(originalSampler, gl_FragCoord.xy).rgb, vec3(gamma)), 1.0);
}
