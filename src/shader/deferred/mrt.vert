#version 450 core
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(binding = 1) uniform ubo_matrix_
{
	mat4 proj;
	mat4 projInv;
	mat4 view;
	mat4 viewInv;
	mat4 projView;
	mat4 projViewRotate;
	vec4 frustumPlanes[6];
	vec2 viewportDim;
}ubo_matrix;

#if defined(ANIM)
layout(binding = 2) uniform ubo_object_animated_
{
	mat4 matrix[8];
}ubo_object;
#else
layout(binding = 2) uniform ubo_object_static_
{
	mat4 matrix[1024];
}ubo_object;
#endif

#if defined(ANIM)
layout(set = 2, binding = 0) uniform ubo_bone_
{
	mat4 matrix[256];
}ubo_bone[8];
#endif

layout(location = 0) in vec3 inVertex;
layout(location = 1) in vec2 inTexcoord;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec3 inTangent;
#if defined(ANIM)
layout(location = 4) in vec4 inBoneWeight;
layout(location = 5) in vec4 inBoneID;
#endif

layout(location = 0) out flat uint outMaterialID;
layout(location = 1) out vec2 outTexcoord;
layout(location = 2) out vec3 outNormal;
layout(location = 3) out vec3 outTangent;

void main()
{
	uint objID = gl_InstanceIndex >> 8;
	outMaterialID = gl_InstanceIndex & 0xff;
	outTexcoord = inTexcoord;
	mat4 modelMatrix = ubo_object.matrix[objID];
#if defined(ANIM)
	mat4 skinMatrix = inBoneWeight[0] * ubo_bone[objID].matrix[int(inBoneID[0])];
	skinMatrix += inBoneWeight[1] * ubo_bone[objID].matrix[int(inBoneID[1])];
	skinMatrix += inBoneWeight[2] * ubo_bone[objID].matrix[int(inBoneID[2])];
	skinMatrix += inBoneWeight[3] * ubo_bone[objID].matrix[int(inBoneID[3])];
	modelMatrix = modelMatrix * skinMatrix;
#endif
	mat3 normalMatrix = transpose(inverse(mat3(ubo_matrix.view * modelMatrix)));
	outNormal = normalize(normalMatrix * inNormal);
	outTangent = normalize(normalMatrix * inTangent);
	gl_Position = ubo_matrix.projView * modelMatrix * vec4(inVertex, 1);
}