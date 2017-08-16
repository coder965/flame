#pragma once

#include "utils.h"
#include "vulkan.h"

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

	struct ResourceBank;
	REFLECTABLE struct Pipeline
	{
		REFL_BANK;

		REFLv std::string name;
		std::string filename;
		std::string filepath;

		REFLv int cx = 0;
		REFLv int cy = 0;

		REFLe VertexInputType vertex_input_type = VertexInputType::zero;
		REFLv int patch_control_points = 0;
		REFLv bool depth_test = false;
		REFLv bool depth_write = false;
		REFLv bool depth_clamp = false;

		REFLe PrimitiveTopology primitive_topology = PrimitiveTopology::triangle_list;
		REFLe PolygonMode polygon_mode = PolygonMode::fill;
		REFLe CullMode cull_mode = CullMode::back;

		std::vector<BlendAttachment> blendAttachments;
		std::vector<DynamicState> dynamicStates;
		std::vector<LinkResource> links;
		Stage *stages[5] = {};

		std::vector<ShaderMacro> shaderMacros;

		VkPrimitiveTopology vkPrimitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		VkPolygonMode vkPolygonMode = VK_POLYGON_MODE_FILL;
		VkCullModeFlagBits vkCullMode = VK_CULL_MODE_BACK_BIT;
		std::vector<VkPipelineColorBlendAttachmentState> vkBlendAttachments;
		std::vector<std::vector<VkDescriptorSetLayoutBinding>> vkDescriptors;
		std::vector<DescriptorSetLayout*> descriptorSetLayouts;
		std::vector<VkPushConstantRange> vkPushConstantRanges;
		PipelineLayout *pipelineLayout = nullptr;

		VkPipelineVertexInputStateCreateInfo *pVertexInputState = nullptr;
		RenderPass *renderPass;
		int subpassIndex;
		std::vector<VkDynamicState> vkDynamicStates;
		std::vector<VkPipelineShaderStageCreateInfo> vkStages;
		VkPipeline pipeline = 0;
		DescriptorSet *descriptorSet = nullptr;

		~Pipeline();
		void loadXML(const std::string &filename);
		void saveXML(const std::string &filename);
		void setup(RenderPass *_renderPass, int _subpassIndex, bool need_default_ds);
		void linkDescriptors(DescriptorSet *set, ResourceBank *resource);
		int descriptorPosition(const std::string &name);
	};
}
