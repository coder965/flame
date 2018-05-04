#pragma once

#include "graphics.h"

#include <flame/string.h>

#include <vector>

namespace flame
{
	namespace graphics
	{
		struct Device;

		struct ShaderResource
		{
			ShaderResourceType type;
			int binding;
			int count;
			std::string name;
		};

		struct ShaderPrivate;

		struct Shader
		{
			MediumString filename;
			std::vector<ShortString> defines;
			ShaderType type;

			ShaderPrivate *_priv;

			inline void add_define(const char *d)
			{
				defines.resize(defines.size() + 1);
				strcpy(defines.back().data, d);
			}

			FLAME_GRAPHICS_EXPORTS void build();
			FLAME_GRAPHICS_EXPORTS void release();
		};

		FLAME_GRAPHICS_EXPORTS Shader *create_shader(Device *d, const char *filename);
		FLAME_GRAPHICS_EXPORTS void destroy_shader(Device *d, Shader *s);
	}
}
