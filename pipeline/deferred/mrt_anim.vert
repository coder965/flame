layout(binding = 0) uniform MATRIX
{
	mat4 proj;
	mat4 projInv;
	mat4 view;
	mat4 viewInv;
	mat4 projView;
	mat4 projViewRotate;
	vec4 frustumPlanes[6];
	vec2 viewportDim;
}u_matrix;

layout(binding = 1) uniform OBJECT
{
	mat4 matrix[1024];
}u_object;

layout(binding = 2) uniform BONE
{
	mat4 matrix[256];
}u_bone[8];

layout(location = 0) in vec3 inVertex;
layout(location = 1) in vec2 inTexcoord;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec3 inTangent;
layout(location = 4) in vec4 inBoneWeight;
layout(location = 5) in vec4 inBoneID;

layout(location = 0) out flat uint outMaterialID;
layout(location = 1) out vec2 outTexcoord;
layout(location = 2) out vec3 outNormal;
layout(location = 3) out vec3 outTangent;

void main()
{
	int objID = gl_InstanceIndex >> 16;
	outMaterialID = gl_InstanceIndex & 0xffff;
	outTexcoord = inTexcoord;
	mat4 modelMatrix = u_object.matrix[objID];
	mat4 skinMatrix = inBoneWeight[0] * u_bone[objID].matrix[int(inBoneID[0])];
	skinMatrix += inBoneWeight[1] * u_bone[objID].matrix[int(inBoneID[1])];
	skinMatrix += inBoneWeight[2] * u_bone[objID].matrix[int(inBoneID[2])];
	skinMatrix += inBoneWeight[3] * u_bone[objID].matrix[int(inBoneID[3])];
	modelMatrix = modelMatrix * skinMatrix;
	mat3 normalMatrix = transpose(inverse(mat3(u_matrix.view * modelMatrix)));
	outNormal = normalize(normalMatrix * inNormal);
	outTangent = normalize(normalMatrix * inTangent);
	gl_Position = u_matrix.projView * modelMatrix * vec4(inVertex, 1);
}