layout(binding = 0) uniform sampler2D originalSampler;
layout(binding = 1) uniform sampler2D miscSampler;
layout(binding = 2) uniform sampler2D uiSampler;

layout(location = 0) out vec4 outColor;

const float gamma = 0.45455;
//const float gamma = 1.0;

void main()
{
	vec4 miscColor = texture(miscSampler, gl_FragCoord.xy);
	vec4 uiColor = texture(uiSampler, gl_FragCoord.xy);
	outColor = vec4((pow(texture(originalSampler, gl_FragCoord.xy).rgb, vec3(gamma)) * (1.0 - miscColor.a) + miscColor.rgb) * uiColor.a + uiColor.rgb, 1.0);
	//outColor = vec4(texture(originalSampler, gl_FragCoord.xy).rgb,1.0);
	//outColor = vec4(1.0);
}
