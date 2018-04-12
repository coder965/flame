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

		enum TextureUsage
		{
			TextureUsageTransferSrc,
			TextureUsageTransferDst,
			TextureUsageShaderSampled,
			TextureUsageAttachment
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

		FLAME_GRAPHICS_EXPORTS Texture *create_texture(Device *d, int cx, int cy, Format format, int usage);
		FLAME_GRAPHICS_EXPORTS Texture *create_texture_from_file(Device *d, const std::string &filename, int usage = 0);
		FLAME_GRAPHICS_EXPORTS void destroy_texture(Device *d, Texture *t);
	}
}

