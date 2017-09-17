#include "pipeline.h"
#include "descriptor.h"
#include "renderpass.h"
#include "sampler.h"
#include "../core.h"
#include "../resource/resource.h"

namespace tke
{
	static VkPipelineVertexInputStateCreateInfo zeroVertexInputState;
	static VkPipelineVertexInputStateCreateInfo plain2dVertexInputState;
	static VkPipelineVertexInputStateCreateInfo vertexInputState;
	static VkPipelineVertexInputStateCreateInfo animatedVertexInputState;
	static VkPipelineVertexInputStateCreateInfo lineVertexInputState;

	PipelineLayout::~PipelineLayout()
	{
		device.mtx.lock();
		vkDestroyPipelineLayout(device.v, v, nullptr);
		device.mtx.unlock();
	}

	static std::vector<PipelineLayout*> pipelineLayouts;

	static VkBlendFactor _vkBlendFactor(BlendFactor f)
	{
		switch (f)
		{
		case BlendFactor::zero:
			return VK_BLEND_FACTOR_ZERO;
		case BlendFactor::one:
			return VK_BLEND_FACTOR_ONE;
		case BlendFactor::src_alpha:
			return VK_BLEND_FACTOR_SRC_ALPHA;
		case BlendFactor::one_minus_src_alpha:
			return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		}
	};

	static VkDescriptorType _vkDescriptorType(DescriptorType t)
	{
		switch (t)
		{
		case DescriptorType::uniform_buffer:
			return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		case DescriptorType::image_n_sampler:
			return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		}
	}

	Pipeline::Pipeline(const std::string &_filename, RenderPass *_renderPass, int _subpassIndex, bool need_default_ds)
	{
		filename = _filename;
		std::experimental::filesystem::path path(filename);
		filepath = path.parent_path().string();
		if (filepath == "")
			filepath = ".";

		{
			AttributeTree at("pipeline");
			at.loadXML(filename);
			at.obtainFromAttributes(this, b);

			for (auto &c : at.children)
			{
				if (c->name == "blend_attachment")
				{
					BlendAttachment ba;
					c->obtainFromAttributes(&ba, ba.b);
					blendAttachments.push_back(ba);
				}
				else if (c->name == "dynamic")
				{
					DynamicState s;
					c->obtainFromAttributes(&s, s.b);
					dynamicStates.push_back(s);
				}
				else if (c->name == "link")
				{
					LinkResource l;
					c->obtainFromAttributes(&l, l.b);
					links.push_back(l);
				}
				else if (c->name == "stage")
				{
					auto s = new Stage(this);
					c->obtainFromAttributes(s, s->b);
					std::experimental::filesystem::path path(s->filename);
					s->filepath = path.parent_path().string();
					if (s->filepath == "")
						s->filepath = ".";
					auto ext = path.extension().string();
					s->type = StageFlagByExt(ext);

					stages[StageIndexByType(s->type)] = s;
				}
				else if (c->name == "macro")
				{
					ShaderMacro m;
					c->obtainFromAttributes(&m, m.b);
					shaderMacros.push_back(m);
				}
			}
		}

		if (!pVertexInputState)
		{
			switch (vertex_input_type)
			{
			case VertexInputType::zero:
				pVertexInputState = &zeroVertexInputState;
				break;
			case VertexInputType::plain2d:
				pVertexInputState = &plain2dVertexInputState;
				break;
			case VertexInputType::normal:
				pVertexInputState = &vertexInputState;
				break;
			case VertexInputType::line:
				pVertexInputState = &lineVertexInputState;
				break;
			case VertexInputType::animated:
				pVertexInputState = &animatedVertexInputState;
				break;
			}
		}

		renderPass = _renderPass;
		subpassIndex = _subpassIndex;

		if (cx == -1)
			cx = tke::resCx;
		if (cy == -1)
			cy = tke::resCy;

		if (cx == 0 && cy == 0)
		{
			vkDynamicStates.push_back(VK_DYNAMIC_STATE_VIEWPORT);
			vkDynamicStates.push_back(VK_DYNAMIC_STATE_SCISSOR);
		}
		for (auto &s : dynamicStates)
		{
			switch (s.type)
			{
			case DynamicStateType::viewport:
				vkDynamicStates.push_back(VK_DYNAMIC_STATE_VIEWPORT);
				break;
			case DynamicStateType::scissor:
				vkDynamicStates.push_back(VK_DYNAMIC_STATE_SCISSOR);
				break;
			}
		}

		switch (primitive_topology)
		{
		case PrimitiveTopology::triangle_list:
			vkPrimitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			break;
		case PrimitiveTopology::line_list:
			vkPrimitiveTopology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
			break;
		case PrimitiveTopology::line_strip:
			vkPrimitiveTopology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
			break;
		case PrimitiveTopology::patch_list:
			vkPrimitiveTopology = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
			break;
		}

		switch (polygon_mode)
		{
		case PolygonMode::fill:
			vkPolygonMode = VK_POLYGON_MODE_FILL;
			break;
		case PolygonMode::line:
			vkPolygonMode = VK_POLYGON_MODE_LINE;
			break;
		}

		switch (cull_mode)
		{
		case CullMode::none:
			vkCullMode = VK_CULL_MODE_NONE;
			break;
		case CullMode::front:
			vkCullMode = VK_CULL_MODE_FRONT_BIT;
			break;
		case CullMode::back:
			vkCullMode = VK_CULL_MODE_BACK_BIT;
			break;
		case CullMode::front_and_back:
			vkCullMode = VK_CULL_MODE_FRONT_AND_BACK;
			break;
		}

		vkBlendAttachments.clear();
		vkDescriptors.clear();
		vkPushConstantRanges.clear();
		vkStages.clear();

		for (auto &b : blendAttachments)
		{
			VkPipelineColorBlendAttachmentState s = {};
			s.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			s.blendEnable = b.enable;
			s.srcColorBlendFactor = _vkBlendFactor(b.src_color);
			s.dstColorBlendFactor = _vkBlendFactor(b.dst_color);
			s.srcAlphaBlendFactor = _vkBlendFactor(b.src_alpha);
			s.dstAlphaBlendFactor = _vkBlendFactor(b.dst_alpha);
			vkBlendAttachments.push_back(s);
		}

		for (auto s : stages)
		{
			if (!s) continue;

			s->create();

			VkPipelineShaderStageCreateInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			info.pName = "main";
			info.stage = (VkShaderStageFlagBits)vkStage(s->type);
			info.module = s->module->v;
			vkStages.push_back(info);

			if (vkDescriptors.size() < s->module->descriptors.size())
			{
				vkDescriptors.resize(s->module->descriptors.size());
				descriptorSetLayouts.resize(s->module->descriptors.size());
			}

			for (auto set = 0; set < s->module->descriptors.size(); set++)
			{
				for (auto &d : s->module->descriptors[set])
				{
					auto found = false;
					for (auto &b : vkDescriptors[set])
					{
						if (b.binding == d.binding)
						{
							b.stageFlags |= vkStage(s->type);
							found = true;
							break;
						}
					}
					if (found) continue;

					VkDescriptorSetLayoutBinding b = {};
					b.descriptorType = _vkDescriptorType(d.type);
					b.binding = d.binding;
					b.descriptorCount = d.count;
					b.stageFlags = vkStage(s->type);
					vkDescriptors[set].push_back(b);
				}
			}
			for (auto &p : s->module->pushConstantRanges)
			{
				auto found = false;
				for (auto &r : vkPushConstantRanges)
				{
					if (r.offset == p.offset & r.size == p.size)
					{
						r.stageFlags |= vkStage(s->type);
						found = true;
						break;
					}
				}
				if (found) continue;

				VkPushConstantRange r = {};
				r.offset = p.offset;
				r.size = p.size;
				r.stageFlags = vkStage(s->type);
				vkPushConstantRanges.push_back(r);
			}
		}

		for (auto set = 0; set < vkDescriptors.size(); set++)
			descriptorSetLayouts[set] = getDescriptorSetLayout(vkDescriptors[set].size(), vkDescriptors[set].data());

		{
			bool found = false;
			for (auto p : pipelineLayouts)
			{
				if (p->descriptorSetLayouts.size() == descriptorSetLayouts.size() && p->pushConstantRanges.size() == vkPushConstantRanges.size())
				{
					bool same = true;
					for (auto i = 0; i < descriptorSetLayouts.size(); i++)
					{
						if (p->descriptorSetLayouts[i] != descriptorSetLayouts[i]->v)
						{
							same = false;
							break;
						}
					}
					for (auto i = 0; i < vkPushConstantRanges.size(); i++)
					{
						auto &pc = p->pushConstantRanges[i];
						if (pc.offset != vkPushConstantRanges[i].offset || pc.size != vkPushConstantRanges[i].size ||
							pc.stageFlags != vkPushConstantRanges[i].stageFlags)
						{
							same = false;
							break;
						}
					}
					if (same)
					{
						pipelineLayout = p;
						found = true;
						break;
					}
				}
			}

			if (!found)
			{
				auto p = new PipelineLayout;
				for (auto d : descriptorSetLayouts)
					p->descriptorSetLayouts.push_back(d->v);
				p->pushConstantRanges.insert(p->pushConstantRanges.begin(), vkPushConstantRanges.begin(), vkPushConstantRanges.end());

				VkPipelineLayoutCreateInfo info = {};
				info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
				info.setLayoutCount = p->descriptorSetLayouts.size();
				info.pSetLayouts = p->descriptorSetLayouts.data();
				info.pushConstantRangeCount = p->pushConstantRanges.size();
				info.pPushConstantRanges = p->pushConstantRanges.data();

				device.mtx.lock();
				auto res = vkCreatePipelineLayout(device.v, &info, nullptr, &p->v);
				assert(res == VK_SUCCESS);
				device.mtx.unlock();

				pipelineLayouts.push_back(p);

				pipelineLayout = p;
			}
		}

		VkPipelineInputAssemblyStateCreateInfo assemblyState = {};
		assemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		assemblyState.topology = vkPrimitiveTopology;
		assemblyState.primitiveRestartEnable = VK_FALSE;

		VkPipelineTessellationStateCreateInfo tessState = {};
		tessState.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
		tessState.patchControlPoints = patch_control_points;

		VkPipelineDepthStencilStateCreateInfo depthStencilState = {};
		depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencilState.depthTestEnable = depth_test;
		depthStencilState.depthWriteEnable = depth_write;
		depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS;

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

		VkPipelineViewportStateCreateInfo viewportState = {};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;
		viewportState.pViewports = &viewport;

		VkPipelineRasterizationStateCreateInfo rasterState = {};
		rasterState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterState.polygonMode = vkPolygonMode;
		rasterState.cullMode = vkCullMode;
		rasterState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterState.depthClampEnable = depth_clamp;
		rasterState.rasterizerDiscardEnable = VK_FALSE;
		rasterState.lineWidth = 1.f;
		rasterState.depthBiasEnable = VK_FALSE;

		VkPipelineColorBlendStateCreateInfo blendState = {};
		if (vkBlendAttachments.size() == 0)
		{
			VkPipelineColorBlendAttachmentState attachment = {};
			attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			vkBlendAttachments.push_back(attachment);
		}

		blendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		blendState.logicOpEnable = VK_FALSE;
		blendState.logicOp = VK_LOGIC_OP_COPY;
		blendState.attachmentCount = vkBlendAttachments.size();
		blendState.pAttachments = vkBlendAttachments.data();

		VkPipelineMultisampleStateCreateInfo multisampleState = {};
		multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampleState.sampleShadingEnable = VK_FALSE;
		multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		VkPipelineDynamicStateCreateInfo dynamicState = {};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = vkDynamicStates.size();
		dynamicState.pDynamicStates = vkDynamicStates.data();

		VkGraphicsPipelineCreateInfo pipelineInfo = {};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.layout = pipelineLayout->v;
		pipelineInfo.stageCount = vkStages.size();
		pipelineInfo.pStages = vkStages.data();
		pipelineInfo.pVertexInputState = pVertexInputState;
		pipelineInfo.pInputAssemblyState = &assemblyState;
		pipelineInfo.pTessellationState = patch_control_points ? &tessState : nullptr;
		pipelineInfo.pDepthStencilState = &depthStencilState;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterState;
		pipelineInfo.pColorBlendState = &blendState;
		pipelineInfo.renderPass = renderPass->v;
		pipelineInfo.subpass = subpassIndex;
		pipelineInfo.pMultisampleState = &multisampleState;
		pipelineInfo.pDynamicState = vkDynamicStates.size() ? &dynamicState : nullptr;

		device.mtx.lock();
		auto res = vkCreateGraphicsPipelines(device.v, 0, 1, &pipelineInfo, nullptr, &pipeline);
		assert(res == VK_SUCCESS);
		device.mtx.unlock();

		if (need_default_ds)
		{
			descriptorSet = new DescriptorSet(this);
			linkDescriptors(descriptorSet, &globalResource);
		}
	}

	Pipeline::~Pipeline()
	{
		pipelineLayout->refCount--;
		if (pipelineLayout->refCount == 0)
		{
			for (auto it = pipelineLayouts.begin(); it != pipelineLayouts.end(); it++)
			{
				if (*it == pipelineLayout)
				{
					pipelineLayouts.erase(it);
					delete pipelineLayout;
					break;
				}
			}
		}

		for (auto d : descriptorSetLayouts)
			releaseDescriptorSetLayout(d);

		delete descriptorSet;

		device.mtx.lock();
		vkDestroyPipeline(device.v, pipeline, nullptr);
		device.mtx.unlock();

		for (int i = 0; i < 5; i++)
			delete stages[i];
	}

	void Pipeline::linkDescriptors(DescriptorSet *set, Resource *resource)
	{
		for (auto &link : links)
		{
			if (link.binding == -1)
			{
				bool found = false;
				for (auto s : stages)
				{
					if (found) break;
					if (!s) continue;
					for (auto set = 0; set < s->module->descriptors.size(); set++)
					{
						for (auto &d : s->module->descriptors[set])
						{
							if (d.name == link.descriptor_name)
							{
								link.binding = d.binding;
								link.type = d.type;
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
			if (link.type == DescriptorType::null)
			{
				bool found = false;
				for (auto s : stages)
				{
					if (found) break;
					if (!s) continue;
					for (auto set = 0; set < s->module->descriptors.size(); set++)
					{
						for (auto &d : s->module->descriptors[set])
						{
							if (d.binding == link.binding)
							{
								link.type = d.type;
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
			case DescriptorType::uniform_buffer:
			{
				auto buffer = resource->getBuffer(link.resource_name);
				if (buffer)
					set->setBuffer(link.binding, link.array_element, buffer);
				else
					printf("%s: unable to link resource %s (binding:%d, type:uniform buffer)\n", filename.c_str(), link.resource_name.c_str(), link.binding);
			}
			break;
			case DescriptorType::image_n_sampler:
			{
				auto image = resource->getImage(link.resource_name);
				if (image)
				{
					if (link.vkSampler == 0)
					{
						switch (link.sampler)
						{
						case SamplerType::none:
							break;
						case SamplerType::plain:
							link.vkSampler = plainSampler;
							break;
						case SamplerType::plain_unnormalized:
							link.vkSampler = plainUnnormalizedSampler;
							break;
						case SamplerType::color:
							link.vkSampler = colorSampler;
							break;
						case SamplerType::color_border:
							link.vkSampler = colorBorderSampler;
							break;
						}
					}
					set->setImage(link.binding, link.array_element, image, link.vkSampler, 0, image->levels.size(), 0, image->layer);
				}
				else
					printf("%s: unable to link resource %s (binding:%d, type:combined image sampler)\n", filename.c_str(), link.resource_name.c_str(), link.binding);
			}
			break;
			}
		}
	}

	int Pipeline::descriptorPosition(const std::string &name)
	{
		for (auto s : stages)
		{
			if (!s) continue;

			for (auto set = 0; set < s->module->descriptors.size(); set++)
			{
				for (auto &d : s->module->descriptors[set])
				{
					if (d.name == name)
						return d.binding;
				}
			}
		}
		return -1;
	}

	static VkPipelineVertexInputStateCreateInfo vertexStateInfo(int bindingCount, VkVertexInputBindingDescription *pBindings, int attributeCount, VkVertexInputAttributeDescription *pAttributes)
	{
		VkPipelineVertexInputStateCreateInfo state = {};
		state.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		state.vertexBindingDescriptionCount = bindingCount;
		state.pVertexBindingDescriptions = pBindings;
		state.vertexAttributeDescriptionCount = attributeCount;
		state.pVertexAttributeDescriptions = pAttributes;

		return state;
	}

	void initPipeline()
	{
		zeroVertexInputState = vertexStateInfo(0, nullptr, 0, nullptr);

		{
			// plain2dVertexInputState will be init in gui
			static VkVertexInputBindingDescription bindings = { 0, sizeof(Vertex2D), VK_VERTEX_INPUT_RATE_VERTEX };

			static VkVertexInputAttributeDescription attributes[] = {
				{ 0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex2D, pos) },
				{ 1, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex2D, uv) },
				{ 2, 0, VK_FORMAT_R8G8B8A8_UNORM, offsetof(Vertex2D, col) }
			};

			plain2dVertexInputState = vertexStateInfo(1, &bindings, ARRAYSIZE(attributes), attributes);
		}

		{
			static VkVertexInputBindingDescription bindings = { 0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX };

			static VkVertexInputAttributeDescription attributes[] = {
				{ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position) },
				{ 1, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv) },
				{ 2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal) },
				{ 3, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, tangent) }
			};

			vertexInputState = vertexStateInfo(1, &bindings, ARRAYSIZE(attributes), attributes);
		}

		{
			static VkVertexInputBindingDescription bindings = { 0, sizeof(VertexAnimated), VK_VERTEX_INPUT_RATE_VERTEX };

			static VkVertexInputAttributeDescription attributes[] = {
				{ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexAnimated, position) },
				{ 1, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(VertexAnimated, uv) },
				{ 2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexAnimated, normal) },
				{ 3, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexAnimated, tangent) },
				{ 4, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(VertexAnimated, boneWeight) },
				{ 5, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(VertexAnimated, boneID) }
			};

			animatedVertexInputState = vertexStateInfo(1, &bindings, ARRAYSIZE(attributes), attributes);
		}

		{
			static VkVertexInputBindingDescription bindings = { 0, sizeof(VertexLine), VK_VERTEX_INPUT_RATE_VERTEX };

			static VkVertexInputAttributeDescription attributes[] = {
				{ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexLine, position) },
				{ 1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexLine, color) }
			};

			lineVertexInputState = vertexStateInfo(1, &bindings, ARRAYSIZE(attributes), attributes);
		}
	}
}
