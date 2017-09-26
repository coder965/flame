#version 450 core
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(binding = 2) uniform OBJECT
{
	mat4 matrix[8];
}u_object;

#if defined(ANIM)
layout(set = 2, binding = 0) uniform BONE
{
	mat4 matrix[256];
}u_bone[8];
#endif

layout(binding = 8) uniform SHADOW
{
	mat4 matrix[8];
}u_shadow;

layout(location = 0) in vec3 inVertex;
layout(location = 1) in vec2 inTexcoord;
#if defined(ANIM)
layout(location = 4) in vec4 inBoneWeight;
layout(location = 5) in vec4 inBoneID;
#endif

layout(location = 0) out vec2 outTexcoord;
layout(location = 1) out flat uint outMaterialID;

void main()
{
	uint objID;
	uint shadowID;
	{
		uint v = gl_InstanceIndex >> 8;
		objID = v & 0x80000;
		shadowID = v >> 20;
		outMaterialID = gl_InstanceIndex & 0xff;
	}
	outTexcoord = inTexcoord;
	mat4 modelMatrix = u_object.matrix[objID];
#if defined(ANIM)
	mat4 skinMatrix = inBoneWeight[0] * u_bone[objID].matrix[int(inBoneID[0])];
	skinMatrix += inBoneWeight[1] * u_bone[objID].matrix[int(inBoneID[1])];
	skinMatrix += inBoneWeight[2] * u_bone[objID].matrix[int(inBoneID[2])];
	skinMatrix += inBoneWeight[3] * u_bone[objID].matrix[int(inBoneID[3])];
	modelMatrix = modelMatrix * skinMatrix;
#endif
	gl_Position = u_shadow.matrix[shadowID] * modelMatrix * vec4(inVertex, 1);
}