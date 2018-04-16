#pragma once

#include <vector>

#include <flame/math.h>
#include <flame/engine/graphics/graphics.h>

namespace flame
{
	struct DescriptorSetLayout;
	struct DescriptorSet;

	struct ResourceLink
	{
		int binding = -1;
		int array_element = 0;
		std::string descriptor_name;
		std::string resource_name;
	};

	struct UniformBufferResourceLink : ResourceLink
	{
		int offset;
		int range;
	};

	struct TextureResourceLink : ResourceLink
	{
		VkSampler sampler;
		int base_level;
		int level_count;
		int base_array;
		int array_count;
		VkImageViewType view_type;
	};

	struct PushConstantRange
	{
		VkShaderStageFlags stage;
		int offset;
		int size;
	};

	bool operator==(const PushConstantRange &lhs, const PushConstantRange &rhs);

	struct PipelineLayout
	{
		std::vector<std::shared_ptr<DescriptorSetLayout>> descriptor_set_layouts;
		std::vector<PushConstantRange> push_constant_ranges;
		VkPipelineLayout v;

		PipelineLayout(const std::vector<std::shared_ptr<DescriptorSetLayout>> &_descriptor_set_layouts, const std::vector<PushConstantRange> &_push_constant_ranges);
		~PipelineLayout();
	};

	std::shared_ptr<PipelineLayout> get_pipeline_layout(const std::vector<std::shared_ptr<DescriptorSetLayout>> &_descriptor_set_layouts, const std::vector<PushConstantRange> &_push_constant_ranges);

	struct PipelineInfo
	{
		std::vector<UniformBufferResourceLink> uniform_buffer_links;
		std::vector<TextureResourceLink> texture_links;

		PipelineInfo();
		PipelineInfo &add_uniform_buffer_link(const std::string &descriptor_name,
			const std::string &resource_name, int array_element = 0,
			int offset = 0, int range = 0);
		PipelineInfo &add_texture_link(const std::string &descriptor_name,
			const std::string &resource_name, int array_element = 0, 
			VkSampler sampler = 0, int base_level = 0, int level_count = 0,
			int base_layer = 0, int layer_count = 0, VkImageViewType view_type = VK_IMAGE_VIEW_TYPE_2D);
	};

	struct Resource;
	struct RenderPass;
	struct Shader;

	struct Pipeline
	{
		PipelineInfo info;

		std::vector<std::shared_ptr<Shader>> shaders;

		std::shared_ptr<PipelineLayout> pipeline_layout;

		RenderPass *render_pass;
		int subpass_index;

		VkPipeline v;

		std::unique_ptr<DescriptorSet> descriptor_set; // set: 0

		// must call in main thread
		Pipeline(const PipelineInfo &_info, RenderPass *_render_pass, int _subpass_index,
			bool need_default_ds = false);
		// must call in main thread
		~Pipeline();
		void create();
		void link_descriptors(DescriptorSet *set, Resource *resource);
		int descriptor_position(const std::string &name);
	};
}
