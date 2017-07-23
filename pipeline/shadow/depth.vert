layout(binding = TKE_UBO_BINDING) uniform MATRIX
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

layout(binding = TKE_UBO_BINDING) uniform OBJECT
{
	mat4 matrix[8];
}u_object;

#if defined(ANIM)
layout(binding = TKE_UBO_BINDING) uniform BONE
{
	mat4 matrix[256];
}u_bone[8];
#endif

layout(location = 0) in vec3 inVertex;
layout(location = 1) in vec2 inTexcoord;
#if defined(ANIM)
layout(location = 4) in vec4 inBoneWeight;
layout(location = 5) in vec4 inBoneID;
#endif

layout(location = 0) out vec2 outTexcoord;

void main()
{
	int objID = gl_InstanceIndex >> 16;
	outTexcoord = inTexcoord;
	mat4 modelMatrix = u_object.matrix[objID];
#if defined(ANIM)
	mat4 skinMatrix = inBoneWeight[0] * u_bone[objID].matrix[int(inBoneID[0])];
	skinMatrix += inBoneWeight[1] * u_bone[objID].matrix[int(inBoneID[1])];
	skinMatrix += inBoneWeight[2] * u_bone[objID].matrix[int(inBoneID[2])];
	skinMatrix += inBoneWeight[3] * u_bone[objID].matrix[int(inBoneID[3])];
	modelMatrix = modelMatrix * skinMatrix;
#endif
	gl_Position = u_matrix.projView * modelMatrix * vec4(inVertex, 1);
}