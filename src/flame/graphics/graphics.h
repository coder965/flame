#pragma once

#ifdef _FLAME_GRAPHICS_EXPORTS
#define FLAME_GRAPHICS_EXPORTS __declspec(dllexport)
#else
#define FLAME_GRAPHICS_EXPORTS __declspec(dllimport)
#endif

#include <functional>

namespace flame
{
	namespace graphics
	{
		struct Format
		{
			enum Value
			{
				R8,
				R16,
				R8G8B8A8,
				R8G8B8A8_SRGB,
				B8G8R8A8,
				R16G16B16A16,
				R16G16B16A16_UNSCALED,
				Depth16,
				Depth32,
				Depth24Stencil8
			};

			enum Type
			{
				TypeColor,
				TypeDepth,
				TypeDepthStencil
			};

			Value v;

			Type get_type()
			{
				if (v == Depth16 || v == Depth32)
					return TypeDepth;
				if (v == Depth24Stencil8)
					return TypeDepthStencil;
				return TypeColor;
			}
		};

		enum MemProp
		{
			MemPropDevice = 1 << 0,
			MemPropHost = 1 << 1,
			MemPropHostCoherent = 1 << 2
		};

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

		enum LightingModel
		{
			LightingModelPhong,
			LightingModelPbr,
			LightingModelPbrIbl
		};
	}
}
