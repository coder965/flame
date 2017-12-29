#pragma once

#include "refl.h"
#include "math/math.h"

namespace tke
{
	IMPL(false) bool only_2d;

	IMPL(0.1f) float near_plane;
	IMPL(1000.f) float far_plane;
	IMPL(60.f) float fovy;

	IMPL() int nowTime;

	IMPL() std::string engine_path;
	IMPL() int res_cx;
	IMPL() int res_cy;
	IMPL() float res_aspect;

	IMPL() glm::mat4 matOrtho;
	IMPL() glm::mat4 matOrthoInv;
	IMPL() glm::mat4 matPerspective;
	IMPL() glm::mat4 matPerspectiveInv;

	enum
	{
		// buffer
		ConstantBufferBinding,
		MaterialBufferBinding,
		MatrixBufferBinding,
		StaticObjectMatrixBufferBinding,
		AnimatedObjectMatrixBufferBinding = 3,
		TerrainBufferBinding,
		WaterBufferBinding,
		LightBufferBinding,
		AmbientBufferBinding,
		ShadowBufferBinding,
		// image
		EnvrImageBinding,
		MainImageBinding,
		DepthImageBinding,
		AlbedoAlphaImageBinding,
		NormalHeightImageBinding,
		SpecRoughnessImageBinding,
		ShadowImageBinding,
		AoImageBinding,
		// terrain special
		TerrainBlendMapBinding
	};

	enum
	{
		TexturesBindingSet = 1,
		BoneBindingSet
	};
}
