#include "graphics_private.h"
#include "device_private.h"

namespace flame
{
	namespace graphics
	{
		VkImageView create_image_view(Device *d, VkImage i, VkFormat format, VkImageAspectFlags aspect,
			VkImageViewType view_type, int base_level, int level_count, int base_layer, int layer_count)
		{
			VkImageViewCreateInfo info;
			info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			info.flags = 0;
			info.pNext = nullptr;
			info.components = {};
			info.image = i;
			info.viewType = view_type;
			info.format = format;
			info.subresourceRange.aspectMask = aspect;
			info.subresourceRange.baseMipLevel = base_level;
			info.subresourceRange.levelCount = level_count;
			info.subresourceRange.baseArrayLayer = base_layer;
			info.subresourceRange.layerCount = layer_count;

			VkImageView v;
			vk_chk_res(vkCreateImageView(d->_priv->device, &info, nullptr, &v));
			return v;
		}

		void destroy_image_view(Device *d, VkImageView v)
		{
			vkDestroyImageView(d->_priv->device, v, nullptr);
		}
	}
}
