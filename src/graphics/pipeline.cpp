#include "../global.h"
#include "image.h"
#include "descriptor.h"
#include "renderpass.h"
#include "renderer.h"
#include "shader.h"
#include "sampler.h"
#include "../resource/resource.h"
#include "pipeline.h"

namespace tke
{
	bool operator==(const PushConstantRange &lhs, const PushConstantRange &rhs)
	{
		return lhs.offset == rhs.offset && lhs.size == rhs.size && lhs.stage == rhs.stage;
	}

	PipelineInfo::PipelineInfo() :
		cx(0),
		cy(0),
		patch_control_points(0),
		depth_test(false),
		depth_write(false),
		depth_clamp(false),
		primitive_topology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST),
		polygon_mode(VK_POLYGON_MODE_FILL),
		cull_mode(VK_CULL_MODE_BACK_BIT)
	{
		vertex_input_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertex_input_state.flags = 0;
		vertex_input_state.pNext = nullptr;
		vertex_input_state.vertexBindingDescriptionCount = 0;
		vertex_input_state.pVertexBindingDescriptions = nullptr;
		vertex_input_state.vertexAttributeDescriptionCount = 0;
		vertex_input_state.pVertexAttributeDescriptions = nullptr;
	}

	PipelineInfo& PipelineInfo::set_cx(int v) 
	{ 
		cx = v; 
		return *this; 
	}

	PipelineInfo& PipelineInfo::set_cy(int v) 
	{ 
		cy = v; 
		return *this; 
	}

	PipelineInfo &PipelineInfo::set_vertex_input_state(const std::initializer_list<std::pair<VertexInputToken, int>> &tokens)
	{
		auto get_size = [](VertexInputToken t)->unsigned int {
			switch (t)
			{
				case TokenF32:
					return sizeof(float);
				case TokenF32V2:
					return sizeof(glm::vec2);
				case TokenF32V3:
					return sizeof(glm::vec3);
				case TokenF32V4:
					return sizeof(glm::vec4);
				case TokenB8V4:
					return sizeof(char) * 4;
			}
			return 0;
		};

		auto get_format = [](VertexInputToken t)->VkFormat {
			switch (t)
			{
				case TokenF32:
					return VK_FORMAT_R32_SFLOAT;
				case TokenF32V2:
					return VK_FORMAT_R32G32_SFLOAT;
				case TokenF32V3:
					return VK_FORMAT_R32G32B32_SFLOAT;
				case TokenF32V4:
					return VK_FORMAT_R32G32B32A32_SFLOAT;
				case TokenB8V4:
					return VK_FORMAT_R8G8B8A8_UNORM;
			}
			return VK_FORMAT_UNDEFINED;
		};
		for (auto &t : tokens)
		{
			auto buf_id = t.second;
			auto buf_index = -1;
			for (int i = 0; i < vertex_input_state_bindings.size(); i++)
			{
				if (vertex_input_state_bindings[i].binding == buf_id)
				{
					buf_index = i;
					break;
				}
			}
			if (buf_index == -1)
			{
				buf_index = vertex_input_state_bindings.size();
				vertex_input_state_bindings.emplace_back();
				vertex_input_state_bindings[buf_index].binding = buf_id;
				vertex_input_state_bindings[buf_index].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			}
			vertex_input_state_attributes.push_back({
				(unsigned int)vertex_input_state_attributes.size(),
				(unsigned int)buf_id,
				get_format(t.first),
				vertex_input_state_bindings[buf_index].stride
				});
			vertex_input_state_bindings[buf_index].stride += get_size(t.first);
		}

		vertex_input_state.vertexBindingDescriptionCount = vertex_input_state_bindings.size();
		vertex_input_state.pVertexBindingDescriptions = vertex_input_state_bindings.data();
		vertex_input_state.vertexAttributeDescriptionCount = vertex_input_state_attributes.size();
		vertex_input_state.pVertexAttributeDescriptions = vertex_input_state_attributes.data();
		return *this;
	}

	PipelineInfo& PipelineInfo::set_patch_control_points(int v) 
	{
		patch_control_points = v; 
		return *this; 
	}

	PipelineInfo& PipelineInfo::set_depth_test(bool v) 
	{ 
		depth_test = v;
		return *this;
	}

	PipelineInfo& PipelineInfo::set_depth_write(bool v) 
	{ 
		depth_write = v; 
		return *this; 
	}

	PipelineInfo& PipelineInfo::set_depth_clamp(bool v) 
	{ 
		depth_clamp = v; 
		return *this; 
	}

	PipelineInfo& PipelineInfo::set_primitive_topology(VkPrimitiveTopology v) 
	{
		primitive_topology = v; 
		return *this; 
	}

	PipelineInfo& PipelineInfo::set_polygon_mode(VkPolygonMode v)
	{ 
		polygon_mode = v;
		return *this; 
	}

	PipelineInfo& PipelineInfo::set_cull_mode(VkCullModeFlagBits v) 
	{ 
		cull_mode = v; 
		return *this; 
	}

	PipelineInfo& PipelineInfo::add_blend_attachment_state(bool enable, VkBlendFactor fsrc_color, VkBlendFactor fdst_color, VkBlendFactor fsrc_alpha, VkBlendFactor fdst_alpha)
	{
		blend_attachment_states.push_back({ enable,
			fsrc_color, fdst_color, VK_BLEND_OP_ADD,
			fsrc_alpha, fdst_alpha, VK_BLEND_OP_ADD,
			VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT });
		return *this;
	}

	PipelineInfo& PipelineInfo::add_dynamic_state(VkDynamicState v)
	{
		dynamic_states.emplace_back(v);
		return *this;
	}

	PipelineInfo& PipelineInfo::add_shader(const std::string &filename,
		const std::initializer_list<std::string> &defines)
	{
		shaders.emplace_back(filename, defines);
		return *this;
	}

	PipelineInfo& PipelineInfo::add_link(const std::string &descriptor_name,
		const std::string &resource_name, int array_element, VkSampler sampler)
	{
		LinkResource l;
		l.descriptor_name = descriptor_name;
		l.resource_name = resource_name;
		l.array_element = array_element;
		l.vkSampler = sampler;
		links.push_back(l);
		return *this;
	}

	PipelineLayout::PipelineLayout(const std::vector<std::shared_ptr<DescriptorSetLayout>> &_descriptor_set_layouts, const std::vector<PushConstantRange> &_push_constant_ranges) :
		descriptor_set_layouts(_descriptor_set_layouts),
		push_constant_ranges(_push_constant_ranges)
	{
		std::vector<VkDescriptorSetLayout> vk_descriptor_set_layouts(descriptor_set_layouts.size());
		for (auto i = 0; i < descriptor_set_layouts.size(); i++)
			vk_descriptor_set_layouts[i] = descriptor_set_layouts[i]->v;

		std::vector<VkPushConstantRange> vk_push_constant_ranges(push_constant_ranges.size());
		for (auto i = 0; i < push_constant_ranges.size(); i++)
		{
			vk_push_constant_ranges[i].offset = push_constant_ranges[i].offset;
			vk_push_constant_ranges[i].size = push_constant_ranges[i].size;
			vk_push_constant_ranges[i].stageFlags = push_constant_ranges[i].stage;
		}

		VkPipelineLayoutCreateInfo info;
		info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		info.flags = 0;
		info.pNext = nullptr;
		info.setLayoutCount = vk_descriptor_set_layouts.size();
		info.pSetLayouts = vk_descriptor_set_layouts.data();
		info.pushConstantRangeCount = vk_push_constant_ranges.size();
		info.pPushConstantRanges = vk_push_constant_ranges.data();

		auto res = vkCreatePipelineLayout(vk_device.v, &info, nullptr, &v);
		assert(res == VK_SUCCESS);
	}

	PipelineLayout::~PipelineLayout()
	{
		vkDestroyPipelineLayout(vk_device.v, v, nullptr);
	}

	static std::vector<std::weak_ptr<PipelineLayout>> pipelineLayouts;

	std::shared_ptr<PipelineLayout> get_or_create_pipeline_layout(const std::vector<std::shared_ptr<DescriptorSetLayout>> &_descriptor_set_layouts, const std::vector<PushConstantRange> &_push_constant_ranges)
	{
		bool found = false;
		for (auto it = pipelineLayouts.begin(); it != pipelineLayouts.end(); )
		{
			auto p = it->lock();

			if (p)
			{
				if (p->descriptor_set_layouts == _descriptor_set_layouts && p->push_constant_ranges == _push_constant_ranges)
					return p;
				it++;
			}
			else
				it = pipelineLayouts.erase(it);
		}

		auto p = std::make_shared <PipelineLayout>(_descriptor_set_layouts, _push_constant_ranges);
		pipelineLayouts.push_back(p);
		return p;
	}

	Pipeline::Pipeline(const PipelineInfo &_info, RenderPass *_render_pass, int _subpass_index, bool need_default_ds) :
		render_pass(_render_pass),
		subpass_index(_subpass_index),
		pipeline(0),
		descriptor_set(nullptr)
	{
		info = _info;

		std::vector<VkPipelineShaderStageCreateInfo> vk_stage_infos;
		std::vector<std::vector<DescriptorSetLayoutBinding>> descriptor_sets;
		std::vector<PushConstantRange> push_constant_ranges;
		std::vector<std::shared_ptr<DescriptorSetLayout>> descriptor_set_layouts;

		for (auto &sdesc : info.shaders)
		{
			auto s = get_or_create_shader(sdesc.first, sdesc.second);

			VkPipelineShaderStageCreateInfo info;
			info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			info.flags = 0;
			info.pNext = nullptr;
			info.pSpecializationInfo = nullptr;
			info.pName = "main";
			info.stage = s->stage;
			info.module = s->vkModule;
			vk_stage_infos.push_back(info);

			if (descriptor_sets.size() < s->descriptor_sets.size())
			{
				descriptor_sets.resize(s->descriptor_sets.size());
				descriptor_set_layouts.resize(s->descriptor_sets.size());
			}

			for (auto set = 0; set < s->descriptor_sets.size(); set++)
			{
				for (auto &d : s->descriptor_sets[set])
				{
					auto found = false;
					for (auto &b : descriptor_sets[set])
					{
						if (b.binding == d->binding)
						{
							b.stage |= s->stage;
							found = true;
							break;
						}
					}
					if (found)
						continue;

					descriptor_sets[set].push_back(d->get_layout_binding(s->stage));
				}
			}
			if (s->push_constant_size > 0)
			{
				auto found = false;
				for (auto &p : push_constant_ranges)
				{
					if (p.size == s->push_constant_size)
					{
						p.stage |= s->stage;
						found = true;
						break;
					}
				}
				if (!found)
				{
					PushConstantRange r;
					r.offset = 0;
					r.size = s->push_constant_size;
					r.stage = s->stage;
					push_constant_ranges.push_back(r);
				}
			}

			shaders.push_back(s);
		}

		if (info.cx == -1)
			info.cx = resolution.x();
		if (info.cy == -1)
			info.cy = resolution.y();

		if (info.cx == 0 && info.cy == 0)
		{
			info.dynamic_states.push_back(VK_DYNAMIC_STATE_VIEWPORT);
			info.dynamic_states.push_back(VK_DYNAMIC_STATE_SCISSOR);
		}

		for (auto set = 0; set < descriptor_sets.size(); set++)
			descriptor_set_layouts[set] = get_or_create_descriptor_set_layout(descriptor_sets[set]);

		pipeline_layout = get_or_create_pipeline_layout(descriptor_set_layouts, push_constant_ranges);

		VkPipelineInputAssemblyStateCreateInfo assemblyState = {};
		assemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		assemblyState.topology = info.primitive_topology;
		assemblyState.primitiveRestartEnable = VK_FALSE;

		VkPipelineTessellationStateCreateInfo tessState = {};
		tessState.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
		tessState.patchControlPoints = info.patch_control_points;

		VkPipelineDepthStencilStateCreateInfo depthStencilState = {};
		depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencilState.depthTestEnable = info.depth_test;
		depthStencilState.depthWriteEnable = info.depth_write;
		depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS;

		VkViewport viewport;
		viewport.width = (float)info.cx;
		viewport.height = (float)info.cy;
		viewport.minDepth = (float)0.0f;
		viewport.maxDepth = (float)1.0f;
		viewport.x = 0;
		viewport.y = 0;

		VkRect2D scissor;
		scissor.extent.width = info.cx;
		scissor.extent.height = info.cy;
		scissor.offset.x = 0;
		scissor.offset.y = 0;

		VkPipelineViewportStateCreateInfo viewportState = {};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;
		viewportState.pViewports = &viewport;

		VkPipelineRasterizationStateCreateInfo rasterState = {};
		rasterState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterState.polygonMode = info.polygon_mode;
		rasterState.cullMode = info.cull_mode;
		rasterState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterState.depthClampEnable = info.depth_clamp;
		rasterState.rasterizerDiscardEnable = VK_FALSE;
		rasterState.lineWidth = 1.f;
		rasterState.depthBiasEnable = VK_FALSE;

		VkPipelineColorBlendStateCreateInfo blendState = {};
		if (info.blend_attachment_states.size() == 0)
			info.add_blend_attachment_state(false);

		blendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		blendState.logicOpEnable = VK_FALSE;
		blendState.logicOp = VK_LOGIC_OP_COPY;
		blendState.attachmentCount = info.blend_attachment_states.size();
		blendState.pAttachments = info.blend_attachment_states.data();

		VkPipelineMultisampleStateCreateInfo multisampleState = {};
		multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampleState.sampleShadingEnable = VK_FALSE;
		multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		VkPipelineDynamicStateCreateInfo dynamicState = {};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = info.dynamic_states.size();
		dynamicState.pDynamicStates = info.dynamic_states.data();

		VkGraphicsPipelineCreateInfo pipelineInfo = {};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.layout = pipeline_layout->v;
		pipelineInfo.stageCount = vk_stage_infos.size();
		pipelineInfo.pStages = vk_stage_infos.data();
		pipelineInfo.pVertexInputState = &info.vertex_input_state;
		pipelineInfo.pInputAssemblyState = &assemblyState;
		pipelineInfo.pTessellationState = info.patch_control_points > 0 ? &tessState : nullptr;
		pipelineInfo.pDepthStencilState = &depthStencilState;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterState;
		pipelineInfo.pColorBlendState = &blendState;
		pipelineInfo.renderPass = render_pass->v;
		pipelineInfo.subpass = subpass_index;
		pipelineInfo.pMultisampleState = &multisampleState;
		pipelineInfo.pDynamicState = info.dynamic_states.size() ? &dynamicState : nullptr;

		auto res = vkCreateGraphicsPipelines(vk_device.v, 0, 1, &pipelineInfo, nullptr, &pipeline);
		assert(res == VK_SUCCESS);

		if (need_default_ds)
		{
			descriptor_set = new DescriptorSet(this);
			link_descriptors(descriptor_set, &globalResource);
		}
	}

	Pipeline::~Pipeline()
	{
		delete descriptor_set;

		vkDestroyPipeline(vk_device.v, pipeline, nullptr);
	}

	void Pipeline::link_descriptors(DescriptorSet *set, Resource *resource)
	{
		std::vector<VkWriteDescriptorSet> writes;
		for (auto &link : info.links)
		{
			if (link.binding == -1)
			{
				bool found = false;
				for (auto &s : shaders)
				{
					if (found) break;

					for (auto set = 0; set < s->descriptor_sets.size(); set++)
					{
						for (auto &d : s->descriptor_sets[set])
						{
							if (d->name == link.descriptor_name)
							{
								link.binding = d->binding;
								link.type = d->type;
								found = true;
								break;
							}
						}
					}
				}
				if (!found)
					int cut = 1;
				//assert(found);
			}
			if (link.type == VK_DESCRIPTOR_TYPE_MAX_ENUM)
			{
				bool found = false;
				for (auto &s : shaders)
				{
					if (found) break;

					for (auto set = 0; set < s->descriptor_sets.size(); set++)
					{
						for (auto &d : s->descriptor_sets[set])
						{
							if (d->binding == link.binding)
							{
								link.type = d->type;
								found = true;
								break;
							}
						}
					}
				}
				if (!found)
					int cut = 1;
				//assert(found);
			}

			switch (link.type)
			{
			case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
			{
				auto buffer = resource->getBuffer(link.resource_name);
				if (buffer)
					writes.push_back(set->bufferWrite(link.binding, link.array_element, buffer));
				else
					printf("unable to link resource %s (binding:%d, type:uniform buffer)\n", link.resource_name.c_str(), link.binding);
			}
				break;
			case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
			{
				auto image = resource->getImage(link.resource_name);
				if (image)
					writes.push_back(set->imageWrite(link.binding, link.array_element, image, link.vkSampler, 0, image->levels.size(), 0, image->layer));
				else
					printf("unable to link resource %s (binding:%d, type:combined image sampler)\n", link.resource_name.c_str(), link.binding);
			}
				break;
			}
		}
		updateDescriptorSets(writes.size(), writes.data());
	}

	int Pipeline::descriptor_position(const std::string &name)
	{
		for (auto &s : shaders)
		{
			for (auto set = 0; set < s->descriptor_sets.size(); set++)
			{
				for (auto &d : s->descriptor_sets[set])
				{
					if (d->name == name)
						return d->binding;
				}
			}
		}
		return -1;
	}
}
