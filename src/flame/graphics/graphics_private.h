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

		VkImageView create_image_view(Device *d, VkImage i, VkFormat format, VkImageAspectFlags aspect, 
			VkImageViewType view_type = VK_IMAGE_VIEW_TYPE_2D, int base_level = 0, int level_count = 1, int base_layer = 0, int layer_count = 1);
		void destroy_image_view(Device *d, VkImageView v);

		inline VkFormat Z(Format f)
		{
			switch (f.v)
			{
				case Format::R8:
					return VK_FORMAT_R8_UNORM;
				case Format::R16:
					return VK_FORMAT_R16_UNORM;
				case Format::R8G8B8A8:
					return VK_FORMAT_R8G8B8A8_UNORM;
				case Format::R8G8B8A8_SRGB:
					return VK_FORMAT_R8G8B8A8_SRGB;
				case Format::B8G8R8A8:
					return VK_FORMAT_B8G8R8A8_UNORM;
				case Format::R16G16B16A16:
					return VK_FORMAT_R16G16B16A16_UNORM;
				case Format::R16G16B16A16_UNSCALED:
					return VK_FORMAT_R16G16B16A16_SFLOAT;
				case Format::Depth16:
					return VK_FORMAT_D16_UNORM;
				case Format::Depth32:
					return VK_FORMAT_D32_SFLOAT;
				case Format::Depth24Stencil8:
					return VK_FORMAT_D24_UNORM_S8_UINT;
				default:
					assert(0);
			}
		}

		inline Format Z(VkFormat f)
		{
			Format ret;

			switch (f)
			{
				case VK_FORMAT_B8G8R8A8_UNORM:
					ret.v = Format::B8G8R8A8;
					break;
				default:
					assert(0);
			}

			return ret;
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
		}
	}
}
