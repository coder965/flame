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

		inline VkShaderStageFlagBits Z(ShaderType t)
		{
			switch (t)
			{
				case ShaderTypeVert:
					return VK_SHADER_STAGE_VERTEX_BIT;
				case ShaderTypeTesc:
					return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
				case ShaderTypeTese:
					return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
				case ShaderTypeGeom:
					return VK_SHADER_STAGE_GEOMETRY_BIT;
				case ShaderTypeFrag:
					return VK_SHADER_STAGE_FRAGMENT_BIT;
			}
		}
	}
}
