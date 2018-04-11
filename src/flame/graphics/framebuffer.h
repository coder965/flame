#pragma once

#include "graphics.h"

namespace flame
{
	namespace graphics
	{
		struct Device;

		struct FramebufferPrivate;
		struct Renderpass;
		struct Textureview;
		struct Swapchain;

		struct Framebuffer
		{
			int cx;
			int cy;

			bool valid;

			FramebufferPrivate *_priv;

			FLAME_GRAPHICS_EXPORTS void set_renderpass(Renderpass *r);
			FLAME_GRAPHICS_EXPORTS void set_view(int index);
			FLAME_GRAPHICS_EXPORTS void set_view_swapchain(int index, Swapchain *s, int image_index);
			FLAME_GRAPHICS_EXPORTS void create();
			FLAME_GRAPHICS_EXPORTS void release();
		};

		FLAME_GRAPHICS_EXPORTS Framebuffer *create_framebuffer(Device *d);
		FLAME_GRAPHICS_EXPORTS void destroy_framebuffer(Device *d, Framebuffer *f);
	}
}

