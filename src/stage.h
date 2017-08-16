#pragma once

#include "utils.h"
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

	REFLECTABLE struct ShaderMacro
	{
		REFL_BANK;

		REFLe StageType stage = StageType::null;
		REFLv std::string value;
	};

	const int StageTypes[] = {
		(int)StageType::vert,
		(int)StageType::tesc,
		(int)StageType::tese,
		(int)StageType::geom,
		(int)StageType::frag
	};

	const std::string StageNames[] = {
		"vert",
		"tesc",
		"tese",
		"geom",
		"frag"
	};

	inline int StageIndexByType(StageType t)
	{
		for (int i = 0; i < 5; i++)
		{
			if (StageTypes[i] == (int)t)
				return i;
		}
		return -1;
	}

	inline const std::string StageNameByType(StageType t)
	{
		return StageNames[StageIndexByType(t)];
	}

	inline StageType StageFlagByExt(const std::string &ext)
	{
		if (ext == ".vert") return StageType::vert;
		if (ext == ".tesc") return StageType::tesc;
		if (ext == ".tese") return StageType::tese;
		if (ext == ".geom") return StageType::geom;
		if (ext == ".frag") return StageType::frag;
		return StageType::null;
	}

	inline VkShaderStageFlags vkStage(StageType f)
	{
		VkShaderStageFlags v = 0;
		if ((int)f & (int)StageType::vert) v |= VK_SHADER_STAGE_VERTEX_BIT;
		if ((int)f & (int)StageType::tesc) v |= VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
		if ((int)f & (int)StageType::tese) v |= VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
		if ((int)f & (int)StageType::geom) v |= VK_SHADER_STAGE_GEOMETRY_BIT;
		if ((int)f & (int)StageType::frag) v |= VK_SHADER_STAGE_FRAGMENT_BIT;
		return v;
	}

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

	struct Pipeline;
	REFLECTABLE struct Stage
	{
		REFL_BANK;

		REFLv std::string filename;
		std::string filepath;
		StageType type;

		Pipeline *parent;
		ShaderModule *module = nullptr;

		Stage(Pipeline *_parent);
		void create();
		~Stage();
	};

}
