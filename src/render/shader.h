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

	struct ShaderModule
	{
		std::string filename;
		std::vector<std::string> defines;

		std::vector<std::vector<Descriptor>> descriptors;
		std::vector<PushConstantRange> pushConstantRanges;
		VkShaderModule v;
		int refCount = 1;

		~ShaderModule();
	};

	struct Stage
	{
		std::string filename;
		StageType type;

		ShaderModule *module = nullptr;

		Stage(const std::string &_filename, const std::vector<std::string> &defines, 
			std::vector<int> &descriptor_set_bindings, const std::vector<std::unique_ptr<Stage>> &siblings);
		~Stage();
	};
}
