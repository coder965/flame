#pragma once

#include "graphics.h"

namespace tke
{
	struct PushConstantRange
	{
		int offset = 0;
		int size = 0;
	};

	struct Descriptor;

	struct Shader
	{
		VkShaderStageFlagBits stage;
		std::string filename;
		std::vector<std::string> defines;
		VkShaderModule vkModule;

		std::vector<std::vector<std::unique_ptr<Descriptor>>> descriptors;
		std::vector<PushConstantRange> pushConstantRanges;

		// must call in main thread
		Shader(const std::string &_filename, const std::vector<std::string> &_defines);
		// must call in main thread
		~Shader();
	};

	extern std::vector<std::weak_ptr<Shader>> loaded_shaders;
}
