#pragma once

#if defined(_WIN64)
typedef __int64 TK_LONG_PTR;
#else
typedef _W64 long TK_LONG_PTR;
#endif
#define TK_ARRAYSIZE(_ARR) ((int)(sizeof(_ARR)/sizeof(*_ARR)))
#define TK_DERIVE_OFFSET(D, B) (TK_LONG_PTR((B*)((D*)1))-1)
#define TK_LOW(I) ((I) & 0xffff)
#define TK_HIGH(I) ((I) >> 16)
#define TK_MAKEINT(H, L) ((L) | ((H) << 16))

namespace tke
{
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
		TerrainNormalHeightMapBinding,
		TerrainBlendMapBinding,
		TerrainColorMapsBinding,
		TerrainNormalMapsBinding
	};

	enum
	{
		TexturesBindingSet = 1,
		BoneBindingSet
	};
}
