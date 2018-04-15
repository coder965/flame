#pragma once

#include <vector>

#include "graphics.h"

namespace flame
{
	namespace graphics
	{
		struct Device;
		struct Swapchain;

		struct RenderpassPrivate;

		struct Renderpass
		{
			bool valid;

			RenderpassPrivate *_priv;

			FLAME_GRAPHICS_EXPORTS void add_attachment_swapchain(Swapchain *s, bool clear);
			FLAME_GRAPHICS_EXPORTS void add_attachment(Format format, bool clear);
			FLAME_GRAPHICS_EXPORTS void add_subpass(const std::initializer_list<int> &color_attachments, int depth_attachment);
			FLAME_GRAPHICS_EXPORTS void add_dependency(int src_subpass, int dst_subpass);
			FLAME_GRAPHICS_EXPORTS void build();
			FLAME_GRAPHICS_EXPORTS void release();
		};

		FLAME_GRAPHICS_EXPORTS Renderpass *create_renderpass(Device *d);
		FLAME_GRAPHICS_EXPORTS void destroy_renderpass(Device *d, Renderpass *r);
	}
}

