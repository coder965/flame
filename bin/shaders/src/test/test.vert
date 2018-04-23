layout(location = 0) in vec3 inVertex;
//layout(location = 1) in vec2 inTexcoord;
layout(location = 1) in vec3 inNormal;

//layout(location = 0) out vec2 outTexcoord;
layout(location = 0) out vec3 outNormal;

layout(binding = 0) uniform ubo_matrix_
{
	mat4 proj;
	mat4 view;
}ubo_matrix;

layout(binding = 1) buffer ubo_matrix_ins_
{
	mat4 model[65536];
}ubo_matrix_ins;

void main()
{
	//outTexcoord = inTexcoord;
	mat4 modelview = ubo_matrix.view * ubo_matrix_ins.model[0];
	mat3 normalMatrix = transpose(inverse(mat3(modelview)));
	outNormal = normalize(normalMatrix * inNormal);
	gl_Position = ubo_matrix.proj * modelview * vec4(inVertex, 1);
}
