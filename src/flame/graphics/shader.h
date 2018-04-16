#pragma once

#include "graphics.h"

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
			std::string filename;
			std::vector<std::string> defines;
			ShaderType type;

			ShaderPrivate *_priv;

			FLAME_GRAPHICS_EXPORTS void build();
			FLAME_GRAPHICS_EXPORTS void release();
		};

		FLAME_GRAPHICS_EXPORTS Shader *create_shader(Device *d, const std::string &filename, const std::vector<std::string> &defines);
		FLAME_GRAPHICS_EXPORTS void destroy_shader(Device *d, Shader *s);
	}
}
