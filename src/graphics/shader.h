#pragma once

#include "graphics.h"

namespace tke
{
	struct Descriptor;

	struct Shader
	{
		VkShaderStageFlagBits stage;
		std::string filename;
		std::vector<std::string> defines;
		VkShaderModule vkModule;

		std::vector<std::vector<std::unique_ptr<Descriptor>>> descriptor_sets;
		int push_constant_size;

		// must call in main thread
		Shader(const std::string &_filename, const std::vector<std::string> &_defines);
		// must call in main thread
		~Shader();
	};

	std::shared_ptr<Shader> get_or_create_shader(const std::string &filename, const std::vector<std::string> &defines);
}
