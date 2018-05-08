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

		struct TexturePrivate;

		struct Texture
		{
			int cx;
			int cy;
			int level;
			int layer;
			Format format;

			TexturePrivate *_priv;
		};

		struct TextureviewPrivate;

		struct Textureview
		{
			TextureviewPrivate *_priv;
		};

		FLAME_GRAPHICS_EXPORTS Texture *create_texture(Device *d, int cx, int cy, int level, int layer, Format::Value format, int usage, int mem_prop);
		FLAME_GRAPHICS_EXPORTS Texture *create_texture_from_file(Device *d, Commandpool *cp, Queue *queue,
			const std::string &filename, int usage = 0, int mem_prop = 0);
		FLAME_GRAPHICS_EXPORTS void destroy_texture(Device *d, Texture *t);

		FLAME_GRAPHICS_EXPORTS Textureview *create_textureview(Device *d, Texture *t,
			TextureViewType view_type = TextureViewType2D, int base_level = 0, int level_count = 1, int base_layer = 0, int layer_count = 1);
		FLAME_GRAPHICS_EXPORTS void destroy_textureview(Device *d, Textureview *v);
	}
}

