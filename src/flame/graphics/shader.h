#pragma once

#include "graphics.h"

namespace tke
{
	struct Descriptor;
	struct Pipeline;

	struct Shader
	{
		VkShaderStageFlagBits stage;
		std::string filename;
		std::vector<std::string> defines;
		VkShaderModule v;

		std::vector<std::vector<std::unique_ptr<Descriptor>>> descriptor_sets;
		int push_constant_size;

		std::vector<Pipeline*> pipelines_use_this;

		// must call in main thread
		Shader(const std::string &_filename, const std::vector<std::string> &_defines);
		// must call in main thread
		~Shader();
		void create();
	};

	std::shared_ptr<Shader> get_or_create_shader(const std::string &filename, const std::vector<std::string> &defines, Pipeline *pipeline = nullptr);
}
