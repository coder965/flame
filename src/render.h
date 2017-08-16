#pragma once

#include <vector>
#include <list>
#include <string>
#include <map>

#include "utils.h"

namespace tke
{
	extern VkFormat swapchainFormat;
	extern VkSampler plainSampler;
	extern VkSampler plainUnnormalizedSampler;
	extern VkSampler colorSampler;
	extern VkSampler colorBorderSampler;
	extern VkSampler colorWrapSampler;

	VkFence createFence();
	void destroyFence(VkFence fence);

	VkEvent createEvent();
	void destroyEvent(VkEvent event);

	VkSemaphore createSemaphore();
	void destroySemaphore(VkSemaphore semaphore);

	void waitFence(VkFence);

	VkPipelineVertexInputStateCreateInfo vertexStateInfo(std::uint32_t bindingCount, VkVertexInputBindingDescription *pBindings, std::uint32_t attributeCount, VkVertexInputAttributeDescription *pAttributes);

	VkAttachmentDescription colorAttachmentDesc(VkFormat format, VkAttachmentLoadOp loadOp);
	VkAttachmentDescription depthAttachmentDesc(VkFormat format, VkAttachmentLoadOp loadOp);
	VkAttachmentDescription swapchainAttachmentDesc(VkAttachmentLoadOp loadOp);
	VkSubpassDescription subpassDesc(int colorCount, VkAttachmentReference *pColors, VkAttachmentReference *pDepth = nullptr, int inputCount = 0, VkAttachmentReference *pInputs = nullptr);
	VkSubpassDependency subpassDependency(int srcSubpass, int dstSubpass);

	Err initRender(bool debug);

	REFLECTABLE enum class Format : int
	{
		null,
		REFLe R8G8B8A8 = 1 << 0,
		REFLe R16G16B16A16 = 1 << 1,
		REFLe R32G32B32A32 = 1 << 2
	};

	inline VkFormat vkFormat(Format f)
	{
		switch (f)
		{
		case Format::R8G8B8A8:
			return VK_FORMAT_R8G8B8A8_UNORM;
		case Format::R16G16B16A16:
			return VK_FORMAT_R16G16B16A16_SFLOAT;
		case Format::R32G32B32A32:
			return VK_FORMAT_R32G32B32A32_SFLOAT;
		}
		return VK_FORMAT_UNDEFINED;
	}

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

	REFLECTABLE enum class AspectFlags : int
	{
		null,
		REFLe color = 1 << 0,
		REFLe depth = 1 << 1,
		REFLe stencil = 1 << 2,
	};

	REFLECTABLE struct PushConstantRange
	{
		REFL_BANK;

		REFLv int offset = 0;
		REFLv int size = 0;
	};

	REFLECTABLE enum class PrimitiveTopology : int
	{
		null,
		REFLe triangle_list = 1 << 0,
		REFLe line_list = 1 << 1,
		REFLe line_strip = 1 << 2,
		REFLe patch_list = 1 << 3
	};

	REFLECTABLE enum class PolygonMode : int
	{
		null,
		REFLe fill = 1 << 0,
		REFLe line = 1 << 1
	};

	REFLECTABLE enum class CullMode : int
	{
		null,
		REFLe none = 1 << 0,
		REFLe front = 1 << 1,
		REFLe back = 1 << 2,
		REFLe front_and_back = 1 << 3
	};

	REFLECTABLE enum class BlendFactor : int
	{
		null,
		REFLe zero = 1 << 0,
		REFLe one = 1 << 1,
		REFLe src_alpha = 1 << 2,
		REFLe one_minus_src_alpha = 1 << 3
	};

	REFLECTABLE struct BlendAttachment
	{
		REFL_BANK;

		REFLv bool enable = false;
		REFLe BlendFactor src_color = BlendFactor::zero;
		REFLe BlendFactor dst_color = BlendFactor::zero;
		REFLe BlendFactor src_alpha = BlendFactor::zero;
		REFLe BlendFactor dst_alpha = BlendFactor::zero;
	};

	REFLECTABLE enum class DescriptorType : int
	{
		null,
		REFLe uniform_buffer = 1 << 0,
		REFLe image_n_sampler = 1 << 1
	};

	REFLECTABLE struct Descriptor
	{
		REFL_BANK;

		REFLe DescriptorType type = DescriptorType::uniform_buffer;
		REFLv int binding = 0;
		REFLv int count = 0;
		REFLv std::string name;
	};

	REFLECTABLE enum class VertexInputType : int
	{
		null,
		REFLe zero = 1 << 0,
		REFLe plain2d = 1 << 1,
		REFLe normal = 1 << 2,
		REFLe animated = 1 << 3,
		REFLe line = 1 << 4
	};

	REFLECTABLE enum class DynamicStateType : int
	{
		null,
		REFLe viewport = 1 << 0,
		REFLe scissor = 1 << 1
	};

	REFLECTABLE struct DynamicState
	{
		REFL_BANK;

		REFLe DynamicStateType type = DynamicStateType::null;
	};

	REFLECTABLE struct ShaderMacro
	{
		REFL_BANK;

		REFLe StageType stage = StageType::null;
		REFLv std::string value;
	};

	struct Model;
	struct ResourceBank
	{
		ResourceBank *parents;

		std::map<std::string, Buffer*> bufferResources;
		std::map<std::string, Image*> imageResources;

		void setBuffer(Buffer *p, const std::string &str);
		void setImage(Image *p, const std::string &str);

		Buffer *getBuffer(const std::string &str);
		Image *getImage(const std::string &str);

		ResourceBank(ResourceBank *_parent);
	};

	extern ResourceBank globalResource;

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

	REFLECTABLE struct Stage
	{
		REFL_BANK;

		REFLv std::string filename;
		std::string filepath;
		StageType type;

		Pipeline *parents;
		ShaderModule *module = nullptr;

		Stage(Pipeline *_parent);
		void create();
		~Stage();
	};

	REFLECTABLE enum class SamplerType : int
	{
		null,
		REFLe none = 1 << 0,
		REFLe plain = 1 << 1,
		REFLe plain_unnormalized = 1 << 2,
		REFLe color = 1 << 3,
		REFLe color_border = 1 << 4
	};

	REFLECTABLE struct LinkResource
	{
		REFL_BANK;

		REFLv int binding = -1;
		REFLv int array_element = 0;
		REFLv std::string descriptor_name;
		REFLv std::string resource_name;
		REFLe SamplerType sampler = SamplerType::none;

		DescriptorType type = DescriptorType::null;
		VkSampler vkSampler = 0;
	};

	struct DescriptorSetLayout
	{
		std::vector<VkDescriptorSetLayoutBinding> bindings;
		VkDescriptorSetLayout v;
		int refCount = 1;

		~DescriptorSetLayout();
	};

	struct PipelineLayout
	{
		std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
		std::vector<VkPushConstantRange> pushConstantRanges;
		VkPipelineLayout v;
		int refCount = 1;

		~PipelineLayout();
	};
}
