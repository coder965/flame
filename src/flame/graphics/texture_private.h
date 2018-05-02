#include "texture.h"
#include "graphics_private.h"

namespace flame
{
	namespace graphics
	{
		struct TexturePrivate
		{
			Device *d;
			VkImage v;
			VkDeviceMemory m;
		};

		struct TextureviewPrivate
		{
			VkImageView v;
		};

		inline VkImageLayout Z(TextureLayout l, Format::Type ft)
		{
			switch (l)
			{
				case TextureLayoutUndefined:
					return VK_IMAGE_LAYOUT_UNDEFINED;
				case TextureLayoutAttachment:
					return ft == Format::TypeColor ?
						VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				case TextureLayoutShaderReadOnly:
					return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				case TextureLayoutShaderStorage:
					return VK_IMAGE_LAYOUT_GENERAL;
				case TextureLayoutTransferSrc:
					return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
				case TextureLayoutTransferDst:
					return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			}
		}

	}
}

