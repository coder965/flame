#pragma once

#include <string>

#include "graphics.h"

namespace flame
{
	namespace graphics
	{
		struct Device;

		struct TextureviewPrivate;

		struct Textureview
		{
			TextureviewPrivate *_priv;
		};

		struct TexturePrivate;

		struct Texture
		{
			int cx;
			int cy;

			TexturePrivate *_priv;

			FLAME_GRAPHICS_EXPORTS Textureview *get_view();
			FLAME_GRAPHICS_EXPORTS void release_view(Textureview *v);
		};

		FLAME_GRAPHICS_EXPORTS Texture *create_texture(Device *d, int cx, int cy, Format format);
		FLAME_GRAPHICS_EXPORTS Texture *create_texture_from_file(Device *d, const std::string &filename);
		FLAME_GRAPHICS_EXPORTS void destroy_texture(Device *d, Texture *t);
	}
}

