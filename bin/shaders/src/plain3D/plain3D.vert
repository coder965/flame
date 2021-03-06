layout(push_constant) uniform PushConstant
{
	mat4 modelview;
	mat4 proj;
	vec4 color;
}pc;

#if defined(ANIM)
layout(binding = 0) uniform ubo_bone_
{
	mat4 matrix[256];
}ubo_bone;
#endif

layout(location = 0) in vec3 inVertex;
#if defined(USE_MATERIAL)
layout(location = 1) in vec2 inTexcoord;
layout(location = 0) out vec2 outTexcoord;
layout(location = 3) out flat uint outMaterialID;
#endif
#if defined(USE_NORMAL)
layout(location = 2) in vec3 inNormal;
layout(location = 1) out vec3 outNormal;
#endif
#if defined(ANIM)
layout(location = 4) in vec4 inBoneWeight;
layout(location = 5) in vec4 inBoneID;
#endif
		
void main()
{
	mat4 modelview = pc.modelview;
#if defined(ANIM)
	mat4 skinMatrix = inBoneWeight[0] * ubo_bone.matrix[int(inBoneID[0])];
	skinMatrix += inBoneWeight[1] * ubo_bone.matrix[int(inBoneID[1])];
	skinMatrix += inBoneWeight[2] * ubo_bone.matrix[int(inBoneID[2])];
	skinMatrix += inBoneWeight[3] * ubo_bone.matrix[int(inBoneID[3])];
	modelview = modelview * skinMatrix;
#endif
#if defined(USE_MATERIAL)
	outTexcoord = inTexcoord;
	outMaterialID = gl_InstanceIndex;
#endif
#if defined(USE_NORMAL)
	mat3 normalMatrix = transpose(inverse(mat3(modelview)));
	outNormal = normalize(normalMatrix * inNormal);
#endif
	gl_Position = pc.proj * modelview * vec4(inVertex, 1);
}