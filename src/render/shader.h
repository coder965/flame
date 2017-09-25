#pragma once

#include "../utils.h"
#include "vulkan.h"
#include "descriptor.h"
#include "push_constant.h"

namespace tke
{
	struct Shader
	{
		VkShaderStageFlagBits stage;
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
