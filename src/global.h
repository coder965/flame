#pragma once

#include "math/math.h"

typedef void(*PF_EVENT0)();
typedef void(*PF_EVENT1)(int);
typedef void(*PF_EVENT2)(int, int);

namespace tke
{
	extern bool only_2d;

	extern float near_plane;
	extern float far_plane;
	extern float fovy;

	extern int nowTime;

	extern std::string engine_path;
	extern int res_cx;
	extern int res_cy;
	extern float res_aspect;

	extern uint32_t FPS;

	enum DescriptorSetBindings
	{
		MainDescriptorSetBinding,
		MaterialDescriptorSetBinding,
		BoneSetDescriptorBinding
	};

	enum MainDescriptorSetBindings
	{
		ConstantBufferDescriptorBinding,
		MatrixBufferDescriptorBinding,
		ObjectMatrixBufferDescriptorBinding,
		TerrainBufferDescriptorBinding,
		TerrainBlendImageDescriptorBinding,
		WaterBufferDescriptorBinding,
		DepthImageDescriptorBinding,
		AlbedoAlphaImageDescriptorBinding,
		NormalHeightImageDescriptorBinding,
		SpecRoughnessDescriptorImageBinding,
		LightBufferDescriptorBinding,
		EnvrImageDescriptorBinding,
		AmbientBufferDescriptorBinding,
		AoImageDescriptorBinding,
		ShadowBufferDescriptorBinding,
		ShadowImageDescriptorBinding
	};

	enum MaterialDescriptorSetBindings
	{
		MaterialBufferDescriptorBinding,
		MaterialImagesDescriptorBinding
	};

	enum Op
	{
		OpNeedRemove,
		OpKeep,
		OpNeedUpdate
	};
}
