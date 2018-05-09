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

		inline VkImageLayout Z(TextureLayout l, Format fmt)
		{
			switch (l)
			{
				case TextureLayoutUndefined:
					return VK_IMAGE_LAYOUT_UNDEFINED;
				case TextureLayoutAttachment:
					if (fmt >= Format_Color_Begin && fmt <= Format_Color_End)
						return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
					else
						return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
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

		inline TextureAspect format_to_aspect(Format fmt)
		{
			if (fmt >= Format_Color_Begin && fmt <= Format_Color_End)
				return TextureAspectColor;
			if (fmt >= Format_Depth_Begin && fmt <= Format_Depth_End)
			{
				int a = TextureAspectDepth;
				if (fmt >= Format_DepthStencil_Begin && fmt <= Format_DepthStencil_End)
					a |= TextureAspectStencil;
				return (TextureAspect)a;
			}
		}

	}
}

