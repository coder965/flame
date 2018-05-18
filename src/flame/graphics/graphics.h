//MIT License
//
//Copyright (c) 2018 wjs
//
//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files (the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions:
//
//The above copyright notice and this permission notice shall be included in all
//copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//SOFTWARE.

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
		enum Format
		{
			Format_R8_UNORM,
			Format_R16_UNORM,
			Format_R8G8B8A8_UNORM,
			Format_R8G8B8A8_SRGB,
			Format_B8G8R8A8_UNORM,
			Format_B8G8R8A8_SRGB,
			Format_Swapchain_B8G8R8A8_UNORM,
			Format_Swapchain_B8G8R8A8_SRGB,
			Format_Swapchain_Begin = Format_Swapchain_B8G8R8A8_UNORM,
			Format_Swapchain_End = Format_Swapchain_B8G8R8A8_SRGB,
			Format_R16G16B16A16_UNORM,
			Format_R16G16B16A16_UNSCALED,
			Format_RGBA_BC3,
			Format_RGBA_ETC2,
			Format_Color_Begin = Format_R8_UNORM,
			Format_Color_End = Format_RGBA_ETC2,

			Format_Depth16,
			Format_Depth32,
			Format_Depth24Stencil8,
			Format_DepthStencil_Begin = Format_Depth24Stencil8,
			Format_DepthStencil_End = Format_Depth24Stencil8,
			Format_Depth_Begin = Format_Depth16,
			Format_Depth_End = Format_Depth24Stencil8,
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

		enum ShaderType
		{
			ShaderNone = 1 << 0,
			ShaderVert = 1 << 1,
			ShaderTesc = 1 << 2,
			ShaderTese = 1 << 3,
			ShaderGeom = 1 << 4,
			ShaderFrag = 1 << 5,
			ShaderComp = 1 << 6
		};

		enum ShaderResourceType
		{
			ShaderResourceUniformbuffer,
			ShaderResourceStoragebuffer,
			ShaderResourceTexture,
			ShaderResourceStorageTexture
		};

		enum PipelineType
		{
			PipelineNone,
			PipelineGraphics,
			PipelineCompute
		};

		enum TextureUsage
		{
			TextureUsageTransferSrc = 1 << 0,
			TextureUsageTransferDst = 1 << 1,
			TextureUsageShaderSampled = 1 << 2,
			TextureUsageShaderStorage = 1 << 3,
			TextureUsageAttachment = 1 << 4
		};

		enum TextureLayout
		{
			TextureLayoutUndefined,
			TextureLayoutAttachment,
			TextureLayoutShaderReadOnly,
			TextureLayoutShaderStorage,
			TextureLayoutTransferSrc,
			TextureLayoutTransferDst
		};

		enum TextureAspect
		{
			TextureAspectColor = 1 << 0,
			TextureAspectDepth = 1 << 1,
			TextureAspectStencil = 1 << 2
		};

		enum TextureViewType
		{
			TextureViewType1D,
			TextureViewType2D,
			TextureViewType3D,
			TextureViewTypeCube,
			TextureViewType1DArray,
			TextureViewType2DArray,
			TextureViewTypeCubeArray
		};

		enum IndiceType
		{
			IndiceTypeUint,
			IndiceTypeUshort
		};

		enum Filter
		{
			FilterNearest,
			FilterLinear
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
