#include <flame/global.h>
#include <flame/graphics/texture.h>
#include <flame/graphics/descriptor.h>
#include <flame/graphics/renderpass.h>
#include <flame/graphics/renderer.h>
#include <flame/graphics/shader.h>
#include <flame/graphics/sampler.h>
#include <flame/resource/resource.h>
#include <flame/graphics/pipeline.h>

namespace flame
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
		for (auto &_v : dynamic_states)
		{
			if (_v == v)
				return *this;
		}
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
		v(0),
		descriptor_set(nullptr)
	{
		info = _info;

		for (auto &_s : info.shaders)
		{
			auto s = get_or_create_shader(_s.first, _s.second, this);
			shaders.push_back(s);
		}

		create();

		if (need_default_ds)
			descriptor_set = std::make_unique<DescriptorSet>(this);

		if (descriptor_set)
			link_descriptors(descriptor_set.get(), &globalResource);
	}

	Pipeline::~Pipeline()
	{
		vkDestroyPipeline(vk_device.v, v, nullptr);
	}

	void Pipeline::create()
	{
		std::vector<VkPipelineShaderStageCreateInfo> vk_stage_infos;
		std::vector<std::vector<DescriptorSetLayoutBinding>> descriptor_sets;
		std::vector<PushConstantRange> push_constant_ranges;
		std::vector<std::shared_ptr<DescriptorSetLayout>> descriptor_set_layouts;

		for (auto &s : shaders)
		{
			VkPipelineShaderStageCreateInfo info;
			info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			info.flags = 0;
			info.pNext = nullptr;
			info.pSpecializationInfo = nullptr;
			info.pName = "main";
			info.stage = s->stage;
			info.module = s->v;
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
		}

		auto cx = info.cx, cy = info.cy;
		if (info.cx == -1)
			cx = resolution.x();
		if (info.cy == -1)
			cy = resolution.y();

		if (info.cx == 0 && info.cy == 0)
		{
			info.add_dynamic_state(VK_DYNAMIC_STATE_VIEWPORT);
			info.add_dynamic_state(VK_DYNAMIC_STATE_SCISSOR);
		}

		for (auto set = 0; set < descriptor_sets.size(); set++)
			descriptor_set_layouts[set] = get_or_create_descriptor_set_layout(descriptor_sets[set]);

		pipeline_layout = get_or_create_pipeline_layout(descriptor_set_layouts, push_constant_ranges);

		VkPipelineInputAssemblyStateCreateInfo assembly_state;
		assembly_state.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		assembly_state.flags = 0;
		assembly_state.pNext = nullptr;
		assembly_state.topology = info.primitive_topology;
		assembly_state.primitiveRestartEnable = VK_FALSE;

		VkPipelineTessellationStateCreateInfo tess_state = {};
		tess_state.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
		tess_state.flags = 0;
		tess_state.pNext = nullptr;
		tess_state.patchControlPoints = info.patch_control_points;

		VkPipelineDepthStencilStateCreateInfo depth_stencil_state;
		depth_stencil_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depth_stencil_state.flags = 0;
		depth_stencil_state.pNext = nullptr;
		depth_stencil_state.depthTestEnable = info.depth_test;
		depth_stencil_state.depthWriteEnable = info.depth_write;
		depth_stencil_state.depthCompareOp = VK_COMPARE_OP_LESS;
		depth_stencil_state.depthBoundsTestEnable = VK_FALSE;
		depth_stencil_state.minDepthBounds = 0;
		depth_stencil_state.maxDepthBounds = 0;
		depth_stencil_state.stencilTestEnable = VK_FALSE;
		depth_stencil_state.front = {};
		depth_stencil_state.back = {};

		VkViewport viewport;
		viewport.width = (float)cx;
		viewport.height = (float)cy;
		viewport.minDepth = (float)0.0f;
		viewport.maxDepth = (float)1.0f;
		viewport.x = 0;
		viewport.y = 0;

		VkRect2D scissor;
		scissor.extent.width = cx;
		scissor.extent.height = cy;
		scissor.offset.x = 0;
		scissor.offset.y = 0;

		VkPipelineViewportStateCreateInfo viewport_state;
		viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewport_state.flags = 0;
		viewport_state.pNext = nullptr;
		viewport_state.viewportCount = 1;
		viewport_state.scissorCount = 1;
		viewport_state.pScissors = &scissor;
		viewport_state.pViewports = &viewport;

		VkPipelineRasterizationStateCreateInfo raster_state;
		raster_state.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		raster_state.flags = 0;
		raster_state.pNext = nullptr;
		raster_state.polygonMode = info.polygon_mode;
		raster_state.cullMode = info.cull_mode;
		raster_state.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		raster_state.depthClampEnable = info.depth_clamp;
		raster_state.rasterizerDiscardEnable = VK_FALSE;
		raster_state.lineWidth = 1.f;
		raster_state.depthBiasEnable = VK_FALSE;
		raster_state.depthBiasClamp = 0.f;
		raster_state.depthBiasConstantFactor = 0.f;
		raster_state.depthBiasSlopeFactor = 0.f;

		if (info.blend_attachment_states.size() == 0)
			info.add_blend_attachment_state(false);

		VkPipelineColorBlendStateCreateInfo blend_state;
		blend_state.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		blend_state.flags = 0;
		blend_state.pNext = nullptr;
		blend_state.blendConstants[0] = 0.f;
		blend_state.blendConstants[1] = 0.f;
		blend_state.blendConstants[2] = 0.f;
		blend_state.blendConstants[3] = 0.f;
		blend_state.logicOpEnable = VK_FALSE;
		blend_state.logicOp = VK_LOGIC_OP_COPY;
		blend_state.attachmentCount = info.blend_attachment_states.size();
		blend_state.pAttachments = info.blend_attachment_states.data();

		VkPipelineMultisampleStateCreateInfo multisample_state;
		multisample_state.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisample_state.flags = 0;
		multisample_state.pNext = nullptr;
		multisample_state.alphaToCoverageEnable = VK_FALSE;
		multisample_state.alphaToOneEnable = VK_FALSE;
		multisample_state.minSampleShading = 0.f;
		multisample_state.pSampleMask = nullptr;
		multisample_state.sampleShadingEnable = VK_FALSE;
		multisample_state.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		VkPipelineDynamicStateCreateInfo dynamic_state = {};
		dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamic_state.flags = 0;
		dynamic_state.pNext = nullptr;
		dynamic_state.dynamicStateCount = info.dynamic_states.size();
		dynamic_state.pDynamicStates = info.dynamic_states.data();

		info.vertex_input_state.vertexBindingDescriptionCount = info.vertex_input_state_bindings.size();
		info.vertex_input_state.pVertexBindingDescriptions = info.vertex_input_state_bindings.data();
		info.vertex_input_state.vertexAttributeDescriptionCount = info.vertex_input_state_attributes.size();
		info.vertex_input_state.pVertexAttributeDescriptions = info.vertex_input_state_attributes.data();

		VkGraphicsPipelineCreateInfo pipeline_info;
		pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipeline_info.flags = 0;
		pipeline_info.pNext = nullptr;
		pipeline_info.basePipelineHandle = 0;
		pipeline_info.basePipelineIndex = 0;
		pipeline_info.layout = pipeline_layout->v;
		pipeline_info.stageCount = vk_stage_infos.size();
		pipeline_info.pStages = vk_stage_infos.data();
		pipeline_info.pVertexInputState = &info.vertex_input_state;
		pipeline_info.pInputAssemblyState = &assembly_state;
		pipeline_info.pTessellationState = info.patch_control_points > 0 ? &tess_state : nullptr;
		pipeline_info.pDepthStencilState = &depth_stencil_state;
		pipeline_info.pViewportState = &viewport_state;
		pipeline_info.pRasterizationState = &raster_state;
		pipeline_info.pColorBlendState = &blend_state;
		pipeline_info.renderPass = render_pass->v;
		pipeline_info.subpass = subpass_index;
		pipeline_info.pMultisampleState = &multisample_state;
		pipeline_info.pDynamicState = info.dynamic_states.size() ? &dynamic_state : nullptr;

		if (v)
			vkDestroyPipeline(vk_device.v, v, nullptr);
		auto res = vkCreateGraphicsPipelines(vk_device.v, 0, 1, &pipeline_info, nullptr, &v);
		assert(res == VK_SUCCESS);
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
