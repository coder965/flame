#pragma once

#include <vector>

#include "../math/math.h"
#include "graphics.h"

namespace tke
{
	struct DescriptorSetLayout;
	struct DescriptorSet;

	struct LinkResource
	{
		int binding = -1;
		int array_element = 0;
		std::string descriptor_name;
		std::string resource_name;

		VkDescriptorType type = VK_DESCRIPTOR_TYPE_MAX_ENUM;
		VkSampler vkSampler = 0;
	};

	struct PipelineLayout
	{
		std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
		std::vector<VkPushConstantRange> pushConstantRanges;
		VkPipelineLayout v;

		~PipelineLayout();
	};

	enum VertexInputToken
	{
		TokenF32,
		TokenF32V2,
		TokenF32V3,
		TokenF32V4,
		TokenB8V4,
	};

	struct PipelineCreateInfo
	{
		int _cx = 0;
		int _cy = 0;
		std::vector<VkVertexInputBindingDescription> _vertex_input_state_bindings;
		std::vector<VkVertexInputAttributeDescription> _vertex_input_state_attributes;
		VkPipelineVertexInputStateCreateInfo _vertex_input_state = {};
		int _patch_control_points = 0;
		bool _depth_test = false;
		bool _depth_write = false;
		bool _depth_clamp = false;
		VkPrimitiveTopology _primitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		VkPolygonMode _polygonMode = VK_POLYGON_MODE_FILL;
		VkCullModeFlagBits _cullMode = VK_CULL_MODE_BACK_BIT;
		std::vector<VkPipelineColorBlendAttachmentState> _blendAttachmentStates;
		std::vector<VkDynamicState> _dynamicStates;
		std::vector<std::pair<std::string, std::vector<std::string>>> _shaders;
		std::vector<LinkResource> links;

		PipelineCreateInfo();
		inline PipelineCreateInfo &cx(int v) { _cx = v; return *this; }
		inline PipelineCreateInfo &cy(int v) { _cy = v; return *this; }
		PipelineCreateInfo &vertex_input_state(const std::initializer_list<std::pair<VertexInputToken, int>> &tokens);
		inline PipelineCreateInfo &patch_control_points(int v) { _patch_control_points = v; return *this; }
		inline PipelineCreateInfo &depth_test(bool v) { _depth_test = v; return *this; }
		inline PipelineCreateInfo &depth_write(bool v) { _depth_write = v; return *this; }
		inline PipelineCreateInfo &depth_clamp(bool v) { _depth_clamp = v; return *this; }
		inline PipelineCreateInfo &primitive_topology(VkPrimitiveTopology v) { _primitiveTopology = v; return *this; }
		inline PipelineCreateInfo &polygon_mode(VkPolygonMode v) { _polygonMode = v; return *this; }
		inline PipelineCreateInfo &cull_mode(VkCullModeFlagBits v) { _cullMode = v; return *this; }
		inline PipelineCreateInfo &add_blend_attachment_state(bool enable,
			VkBlendFactor fsrc_color = VK_BLEND_FACTOR_ONE, VkBlendFactor fdst_color = VK_BLEND_FACTOR_ZERO,
			VkBlendFactor fsrc_alpha = VK_BLEND_FACTOR_ONE, VkBlendFactor fdst_alpha = VK_BLEND_FACTOR_ZERO)
		{
			_blendAttachmentStates.push_back({enable,
				fsrc_color, fdst_color, VK_BLEND_OP_ADD,
				fsrc_alpha, fdst_alpha, VK_BLEND_OP_ADD,
				VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT});
			return *this;
		}
		inline PipelineCreateInfo &add_dynamic_state(VkDynamicState v) 
		{ 
			_dynamicStates.emplace_back(v);
			return *this; 
		}
		inline PipelineCreateInfo &add_shader(const std::string &filename, 
			const std::initializer_list<std::string> &defines)
		{
			_shaders.emplace_back(filename, defines);
			return *this;
		}
		inline PipelineCreateInfo &add_link(const std::string &descriptor_name,
			const std::string &resource_name, int array_element = 0, VkSampler sampler = 0)
		{
			LinkResource l;
			l.descriptor_name = descriptor_name;
			l.resource_name = resource_name;
			l.array_element = array_element;
			l.vkSampler = sampler;
			links.push_back(l);
			return *this;
		}
	};

	struct Resource;
	struct RenderPass;
	struct Shader;

	struct Pipeline
	{
		std::vector<LinkResource> links;
		std::vector<std::shared_ptr<Shader>> shaders;

		std::vector<std::shared_ptr<DescriptorSetLayout>> descriptorSetLayouts;
		std::shared_ptr<PipelineLayout> pipelineLayout;

		VkPipeline pipeline = 0;
		DescriptorSet *descriptorSet = nullptr;

		// must call in main thread
		Pipeline(PipelineCreateInfo &info, RenderPass *_renderPass, int _subpassIndex, 
			bool need_default_ds = false);
		// must call in main thread
		~Pipeline();
		void linkDescriptors(DescriptorSet *set, Resource *resource);
		int descriptorPosition(const std::string &name);
	};
}
