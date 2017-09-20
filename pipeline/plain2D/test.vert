layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aUV;
layout(location = 2) in vec4 aColor;

layout(push_constant) uniform uPushConstant
{
	vec2 uScale;
	vec2 uTranslate;
}pc;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec2 outUV;
layout(location = 2) out flat uint outID;

void main()
{
	outColor = vec4(pc.uScale.x / 2000, pc.uScale.y / 2000, 0, 1);
	outColor = vec4(aPos, 0, 1);
	outUV = aUV;
	outID = gl_InstanceIndex;

	//gl_Position = vec4(aPos * pc.uScale + pc.uTranslate, 0, 1);
	//gl_Position = vec4(aPos * vec2(2 / 1900, 2 / 1000) + vec2(-1, -1), 0, 1);

	vec2 v[3] = {
		vec2(0, 0),
		vec2(0, 1),
		vec2(1, 0),
	};
	gl_Position = vec4(v[gl_VertexIndex / 2 % 3], 0, 1);
}