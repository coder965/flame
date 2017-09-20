#pragma once

#include "../utils.h"
#include "vulkan.h"
#include "descriptor.h"
#include "push_constant.h"

namespace tke
{
	REFLECTABLE enum class StageType : int
	{
		null,
		REFLe vert = 1 << 0,
		REFLe tesc = 1 << 1,
		REFLe tese = 1 << 2,
		REFLe geom = 1 << 3,
		REFLe frag = 1 << 4
	};

	struct Shader
	{
		StageType type;
		std::string filename;
		std::vector<std::string> defines;
		VkShaderModule vkModule;

		std::vector<std::vector<Descriptor>> descriptors;
		std::vector<PushConstantRange> pushConstantRanges;

		Shader(const std::string &_filename, const std::vector<std::string> &_defines);
		~Shader();
	};

	IMPL() std::vector<std::weak_ptr<Shader>> loaded_shaders;
}
