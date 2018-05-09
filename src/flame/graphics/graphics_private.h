#pragma once

#define NOMINMAX
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#undef INFINITE

#include <list>
#include <assert.h>

#include "graphics.h"

namespace flame
{
	namespace graphics
	{
		struct Device;

		inline void vk_chk_res(VkResult res)
		{
			assert(res == VK_SUCCESS);
		}

		VkImageView create_imageview(Device *d, VkImage i, VkFormat format, VkImageAspectFlags aspect, 
			VkImageViewType view_type = VK_IMAGE_VIEW_TYPE_2D, int base_level = 0, int level_count = 1, int base_layer = 0, int layer_count = 1);
		void destroy_imageview(Device *d, VkImageView v);

		inline VkFormat Z(Format f)
		{
			switch (f)
			{
				case Format_R8_UNORM:
					return VK_FORMAT_R8_UNORM;
				case Format_R16_UNORM:
					return VK_FORMAT_R16_UNORM;
				case Format_R8G8B8A8_UNORM:
					return VK_FORMAT_R8G8B8A8_UNORM;
				case Format_R8G8B8A8_SRGB:
					return VK_FORMAT_R8G8B8A8_SRGB;
				case Format_B8G8R8A8_UNORM: case Format_Swapchain_B8G8R8A8_UNORM:
					return VK_FORMAT_B8G8R8A8_UNORM;
				case Format_B8G8R8A8_SRGB: case Format_Swapchain_B8G8R8A8_SRGB:
					return VK_FORMAT_B8G8R8A8_SRGB;
				case Format_R16G16B16A16_UNORM:
					return VK_FORMAT_R16G16B16A16_UNORM;
				case Format_R16G16B16A16_UNSCALED:
					return VK_FORMAT_R16G16B16A16_SFLOAT;
				case Format_RGBA_BC3:
					return VK_FORMAT_BC3_UNORM_BLOCK;
				case Format_RGBA_ETC2:
					return VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK;

				case Format_Depth16:
					return VK_FORMAT_D16_UNORM;
				case Format_Depth32:
					return VK_FORMAT_D32_SFLOAT;
				case Format_Depth24Stencil8:
					return VK_FORMAT_D24_UNORM_S8_UINT;

				default:
					assert(0);
			}
		}

		inline Format Z(VkFormat f, bool is_swapchain)
		{
			switch (f)
			{
				case VK_FORMAT_B8G8R8A8_UNORM:
					return is_swapchain ? Format_Swapchain_B8G8R8A8_UNORM : Format_B8G8R8A8_UNORM;
				case VK_FORMAT_B8G8R8A8_SRGB:
					return is_swapchain ? Format_Swapchain_B8G8R8A8_SRGB : Format_B8G8R8A8_SRGB;

				default:
					assert(0);
			}
		}

		inline VkMemoryPropertyFlags Z(MemProp p)
		{
			VkMemoryPropertyFlags vk_mem_prop = 0;
			if (p & MemPropDevice)
				vk_mem_prop |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
			if (p & MemPropHost)
				vk_mem_prop |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
			if (p & MemPropHostCoherent)
				vk_mem_prop |= VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
			return vk_mem_prop;
		}

		inline VkDescriptorType Z(ShaderResourceType t)
		{
			switch (t)
			{
				case ShaderResourceUniformbuffer:
					return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				case ShaderResourceStoragebuffer:
					return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
				case ShaderResourceTexture:
					return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				case ShaderResourceStorageTexture:
					return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
			}
		}

		inline VkShaderStageFlags Z(ShaderType t)
		{
			VkShaderStageFlags vk_shader_stage = 0;
			if (t & ShaderVert)
				vk_shader_stage |= VK_SHADER_STAGE_VERTEX_BIT;
			if (t & ShaderTesc)
				vk_shader_stage |= VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
			if (t & ShaderTese)
				vk_shader_stage |= VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
			if (t & ShaderGeom)
				vk_shader_stage |= VK_SHADER_STAGE_GEOMETRY_BIT;
			if (t & ShaderFrag)
				vk_shader_stage |= VK_SHADER_STAGE_FRAGMENT_BIT;
			if (t & ShaderComp)
				vk_shader_stage |= VK_SHADER_STAGE_COMPUTE_BIT;
			return vk_shader_stage;
		}

		inline VkShaderStageFlagBits Z(VkShaderStageFlags f)
		{
			return VkShaderStageFlagBits(f);
		}

		inline VkPipelineBindPoint Z(PipelineType t)
		{
			switch (t)
			{
				case PipelineNone:
					return VkPipelineBindPoint(-1);
				case PipelineGraphics:
					return VK_PIPELINE_BIND_POINT_GRAPHICS;
				case PipelineCompute:
					return VK_PIPELINE_BIND_POINT_COMPUTE;
			}
		}

		inline VkImageUsageFlags Z(TextureUsage u, Format fmt)
		{
			VkImageUsageFlags vk_usage = 0;
			if (u & TextureUsageTransferSrc)
				vk_usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
			if (u & TextureUsageTransferDst)
				vk_usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
			if (u & TextureUsageShaderSampled)
				vk_usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
			if (u & TextureUsageShaderStorage)
				vk_usage |= VK_IMAGE_USAGE_STORAGE_BIT;
			if (u & TextureUsageAttachment)
			{
				if (fmt >= Format_Color_Begin && fmt <= Format_Color_End)
					vk_usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
				else
					vk_usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			}
			return vk_usage;
		}

		inline VkImageAspectFlags Z(TextureAspect a)
		{
			VkImageAspectFlags vk_aspect = 0;
			if (a & TextureAspectColor)
				vk_aspect |= VK_IMAGE_ASPECT_COLOR_BIT;
			if (a & TextureAspectDepth)
				vk_aspect |= VK_IMAGE_ASPECT_DEPTH_BIT;
			if (a & TextureAspectStencil)
				vk_aspect |= VK_IMAGE_ASPECT_STENCIL_BIT;
			return vk_aspect;
		}

		inline VkImageViewType Z(TextureViewType t)
		{
			switch (t)
			{
				case TextureViewType1D:
					return VK_IMAGE_VIEW_TYPE_1D;
				case TextureViewType2D:
					return VK_IMAGE_VIEW_TYPE_2D;
				case TextureViewType3D:
					return VK_IMAGE_VIEW_TYPE_3D;
				case TextureViewTypeCube:
					return VK_IMAGE_VIEW_TYPE_CUBE;
				case TextureViewType1DArray:
					return VK_IMAGE_VIEW_TYPE_1D_ARRAY;
				case TextureViewType2DArray:
					return VK_IMAGE_VIEW_TYPE_2D_ARRAY;
				case TextureViewTypeCubeArray:
					return VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
			}
		}

		inline VkFilter Z(Filter f)
		{
			switch (f)
			{
				case FilterNearest:
					return VK_FILTER_NEAREST;
				case FilterLinear:
					return VK_FILTER_LINEAR;
			}
		}
	}
}
