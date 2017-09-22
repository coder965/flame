#pragma once

#include "../utils.h"
#include "../math/math.h"
#include "vulkan.h"
#include "shader.h"
#include "sampler.h"

namespace tke
{
	struct Vertex2D
	{
		glm::vec2       pos;
		glm::vec2       uv;
		std::uint32_t   col;
	};

	struct Vertex
	{
		glm::vec3 position;
		glm::vec2 uv;
		glm::vec3 normal;
		glm::vec3 tangent;
	};

	struct VertexAnimated
	{
		glm::vec3 position;
		glm::vec2 uv;
		glm::vec3 normal;
		glm::vec3 tangent;

		glm::vec4 boneWeight;
		glm::vec4 boneID;
	};

	struct VertexLine
	{
		glm::vec3 position;
		glm::vec3 color;
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

	struct PipelineLayout
	{
		std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
		std::vector<VkPushConstantRange> pushConstantRanges;
		VkPipelineLayout v;

		~PipelineLayout();
	};

	IMPL() VkPipelineVertexInputStateCreateInfo zeroVertexInputState;
	IMPL() VkPipelineVertexInputStateCreateInfo plain2dVertexInputState;
	IMPL() VkPipelineVertexInputStateCreateInfo vertexInputState;
	IMPL() VkPipelineVertexInputStateCreateInfo animatedVertexInputState;
	IMPL() VkPipelineVertexInputStateCreateInfo lineVertexInputState;

	struct PipelineCreateInfo
	{
		int _cx = 0;
		int _cy = 0;
		VkPipelineVertexInputStateCreateInfo *pVertexInputState = &zeroVertexInputState;

		PipelineCreateInfo &cx(int v);
		PipelineCreateInfo &cy(int v);
	};

	struct Resource;
	struct RenderPass;
	REFLECTABLE struct Pipeline
	{
		REFL_BANK;

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

		std::vector<LinkResource> links;
		std::vector<std::shared_ptr<Shader>> shaders;

		VkPrimitiveTopology vkPrimitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		VkPolygonMode vkPolygonMode = VK_POLYGON_MODE_FILL;
		VkCullModeFlagBits vkCullMode = VK_CULL_MODE_BACK_BIT;
		std::vector<std::shared_ptr<DescriptorSetLayout>> descriptorSetLayouts;
		std::shared_ptr<PipelineLayout> pipelineLayout;

		VkPipelineVertexInputStateCreateInfo *pVertexInputState = nullptr;
		RenderPass *renderPass;
		int subpassIndex;
		VkPipeline pipeline = 0;
		DescriptorSet *descriptorSet = nullptr;

		Pipeline(const std::string &_filename, RenderPass *_renderPass, int _subpassIndex, bool need_default_ds = false);
		~Pipeline();
		void linkDescriptors(DescriptorSet *set, Resource *resource);
		int descriptorPosition(const std::string &name);
	};

	void initPipeline();
}
