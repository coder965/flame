#pragma once

#include <string>

#include "graphics.h"

namespace flame
{
	namespace graphics
	{
		struct Device;
		struct Commandpool;
		struct Commandbuffer;
		struct Queue;

		struct TextureviewPrivate;

		struct Textureview
		{
			TextureviewPrivate *_priv;
		};

		enum TextureUsage
		{
			TextureUsageTransferSrc = 1 << 0,
			TextureUsageTransferDst = 1 << 1,
			TextureUsageShaderSampled = 1 << 2,
			TextureUsageAttachment = 1 << 3
		};

		enum TextureLayout
		{
			TextureLayoutUndefined,
			TextureLayoutAttachment,
			TextureLayoutShaderReadOnlyOptimal,
			TextureLayoutTransferSrc,
			TextureLayoutTransferDst
		};

		struct TexturePrivate;

		struct Texture
		{
			int cx;
			int cy;
			int level;
			int layer;
			Format format;

			TexturePrivate *_priv;

			FLAME_GRAPHICS_EXPORTS void change_layout(Commandbuffer *cb, TextureLayout from, TextureLayout to, 
				int base_level = 0, int level_count = 0, int base_layer = 0, int layer_count = 0);
			FLAME_GRAPHICS_EXPORTS Textureview *get_view();
			FLAME_GRAPHICS_EXPORTS void release_view(Textureview *v);
		};

		FLAME_GRAPHICS_EXPORTS Texture *create_texture(Device *d, int cx, int cy, int level, int layer, Format format, int usage, int mem_prop = 0);
		FLAME_GRAPHICS_EXPORTS Texture *create_texture_from_file(Device *d, Commandpool *cp, Queue *queue,
			const std::string &filename, int usage = 0, int mem_prop = 0);
		FLAME_GRAPHICS_EXPORTS void destroy_texture(Device *d, Texture *t);
	}
}

