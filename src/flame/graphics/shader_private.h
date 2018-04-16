#pragma once

#include "shader.h"
#include "graphics_private.h"

#include <list>

namespace flame
{
	namespace graphics
	{
		struct Device;

		struct ShaderPrivate
		{
			std::vector<std::list<ShaderResource>> resources;
			int push_constant_size;

			Device *d;
			VkShaderModule v;
		};
	}
}
