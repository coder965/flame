#include "texture_private.h"
#include "device_private.h"

namespace flame
{
	namespace graphics
	{
		Texture *create_texture(Device *d, int cx, int cy, Format format, int usage)
		{
			auto t = new Texture;
			t->cx = cx;
			t->cy = cy;
			t->_priv = new TexturePrivate;
			t->_priv->d = d;

			auto format_type = format.get_type();
			
			VkImageUsageFlags vk_usage = 0;
			if (usage & TextureUsageTransferSrc)
				vk_usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
			if (usage & TextureUsageTransferDst)
				vk_usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
			if (usage & TextureUsageShaderSampled)
				vk_usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
			if (usage & TextureUsageAttachment)
			{
				if (format_type == Format::TypeColor)
					vk_usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
				else
					vk_usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			}

			VkImageCreateInfo imageInfo;
			imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			imageInfo.flags = 0;
			imageInfo.pNext = nullptr;
			imageInfo.imageType = VK_IMAGE_TYPE_2D;
			imageInfo.extent.width = cx;
			imageInfo.extent.height = cy;
			imageInfo.extent.depth = 1;
			imageInfo.mipLevels = 1;
			imageInfo.arrayLayers = 1;
			imageInfo.format = Z(format);
			imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
			imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			imageInfo.usage = usage;
			imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
			imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			imageInfo.queueFamilyIndexCount = 0;
			imageInfo.pQueueFamilyIndices = nullptr;

			vk_chk_res(vkCreateImage(d->_priv->device, &imageInfo, nullptr, &t->_priv->v));

			VkMemoryRequirements memRequirements;
			vkGetImageMemoryRequirements(d->_priv->device, t->_priv->v, &memRequirements);

			VkMemoryAllocateInfo allocInfo;
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.pNext = nullptr;
			allocInfo.allocationSize = memRequirements.size;
			allocInfo.memoryTypeIndex = find_memory_type(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

			vk_chk_res(vkAllocateMemory(d->_priv->device, &allocInfo, nullptr, &t->_priv->m));

			vk_chk_res(vkBindImageMemory(d->_priv->device, t->_priv->v, t->_priv->m, 0));

			return nullptr;
		}

		Texture *create_texture_from_file(Device *d, const std::string &filename, int usage)
		{

		}

		void destroy_texture(Device *d, Texture *t)
		{

		}
	}
}

