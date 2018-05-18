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

