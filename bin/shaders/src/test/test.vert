layout(location = 0) in vec3 inVertex;
layout(location = 1) in vec2 inTexcoord;
layout(location = 2) in vec3 inNormal;

layout(location = 0) out vec2 outTexcoord;
layout(location = 1) out vec3 outNormal;

layout(binding = 0) uniform ubo_matrix_
{
	mat4 proj;
	mat4 view;
	mat4 model;
}ubo_matrix;

void main()
{
	outTexcoord = inTexcoord;
	mat3 normalMatrix = transpose(inverse(mat3(ubo_matrix.view * ubo_matrix.model)));
	outNormal = normalize(normalMatrix * inNormal);
	//outNormal = normalize(inNormal);
	gl_Position = ubo_matrix.proj * ubo_matrix.view * ubo_matrix.model * vec4(inVertex, 1);
}
