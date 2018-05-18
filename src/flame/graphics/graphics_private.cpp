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

#include "graphics_private.h"
#include "device_private.h"

namespace flame
{
	namespace graphics
	{
		VkImageView create_imageview(Device *d, VkImage i, VkFormat format, VkImageAspectFlags aspect,
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

		void destroy_imageview(Device *d, VkImageView v)
		{
			vkDestroyImageView(d->_priv->device, v, nullptr);
		}
	}
}
