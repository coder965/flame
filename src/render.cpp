#include <experimental/filesystem>
#include <regex>

#include "core.h"
#include "gui.h"
#include "render.h"
#include "model.h"
#include "image.file.h"

namespace tke
{
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
		case DescriptorType::storage_buffer:
			return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		case DescriptorType::storage_image:
			return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		case DescriptorType::image_n_sampler:
			return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		case DescriptorType::input_attachment:
			return VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		}
	}

	Stage::Stage(Pipeline *_parent)
	{
		parent = _parent;
	}

	Stage::~Stage()
	{
	}

	void Pipeline::destroy()
	{
		for (int i = 0; i < 5; i++) 
			delete stages[i];
	}

	int Pipeline::descriptorPosition(const std::string &name)
	{
		for (auto s : stages)
		{
			if (!s) continue;

			for (auto &d : s->descriptors)
			{
				if (d.name == name)
					return d.binding;
			}
		}
		assert(false);
		return -1;
	}

	void Pipeline::create(const std::string &_filename, VkPipelineVertexInputStateCreateInfo *pVertexInputState, VkRenderPass renderPass, std::uint32_t subpassIndex)
	{
		pipelineLoadXML<Pipeline, Stage>(this, _filename);

		m_pVertexInputState = pVertexInputState;

		m_renderPass = renderPass;
		m_subpassIndex = subpassIndex;

		if (cx == -1)
			cx = tke::resCx;
		if (cy == -1)
			cy = tke::resCy;

		if (cx == 0 && cy == 0)
		{
			m_dynamics.push_back(VK_DYNAMIC_STATE_VIEWPORT);
			m_dynamics.push_back(VK_DYNAMIC_STATE_SCISSOR);
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

			VkPipelineShaderStageCreateInfo i = {};
			i.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			i.pName = "main";
			i.stage = (VkShaderStageFlagBits)vkStage(s->type);
			i.module = createShaderModule(filepath + "/" + s->filename + ".spv");
			vkStages.push_back(i);

			for (auto &d : s->descriptors)
			{
				auto found = false;
				for (auto &b : vkDescriptors)
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
				vkDescriptors.push_back(b);
			}
			for (auto &p : s->pushConstantRanges)
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

		m_descriptorSetLayout = createDescriptorSetLayout(vkDescriptors);
		m_pipelineLayout = createPipelineLayout(m_descriptorSetLayout, vkPushConstantRanges);

		if (m_pipeline) vk::destroyPipeline(m_pipeline);

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
		dynamicState.dynamicStateCount = m_dynamics.size();
		dynamicState.pDynamicStates = m_dynamics.data();

		VkGraphicsPipelineCreateInfo pipelineInfo = {};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.layout = m_pipelineLayout;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
		pipelineInfo.stageCount = vkStages.size();
		pipelineInfo.pStages = vkStages.data();
		pipelineInfo.pVertexInputState = m_pVertexInputState;
		pipelineInfo.pInputAssemblyState = &assemblyState;
		pipelineInfo.pTessellationState = patch_control_points ? &tessState : nullptr;
		pipelineInfo.pDepthStencilState = &depthStencilState;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterState;
		pipelineInfo.pColorBlendState = &blendState;
		pipelineInfo.renderPass = renderPass;
		pipelineInfo.subpass = subpassIndex;
		pipelineInfo.pMultisampleState = &multisampleState;
		pipelineInfo.pDynamicState = m_dynamics.size() ? &dynamicState : nullptr;
		m_pipeline = vk::createPipeline(&pipelineInfo);

		if (m_descriptorSet) vk::descriptorPool.free(m_descriptorSet);
		m_descriptorSet = vk::descriptorPool.allocate(&m_descriptorSetLayout);

		for (auto &link : links)
		{
			DescriptorType type = DescriptorType::null;

			if (link.binding == -1)
			{
				bool found = false;
				for (auto s : stages)
				{
					if (!s) continue;

					if (found) break;
					for (auto &d : s->descriptors)
					{
						if (d.name == link.descriptor_name)
						{
							link.binding = d.binding;
							type = d.type;
							found = true;
							break;
						}
					}
				}
				//assert(found);
			}
			else
			{
				bool found = false;
				for (auto s : stages)
				{
					if (!s) continue;

					if (found) break;
					for (auto &d : s->descriptors)
					{
						if (d.binding == link.binding)
						{
							type = d.type;
							found = true;
							break;
						}
					}
				}
				if (!found) continue;
				//assert(found);
			}

			switch (type)
			{
			case DescriptorType::uniform_buffer:
			{
				auto pUniformBuffer = (UniformBuffer*)pResource->getBuffer(link.resource_name);
				if (pUniformBuffer)
					vk::descriptorPool.addWrite(m_descriptorSet, _vkDescriptorType(type), link.binding, &pUniformBuffer->m_info, link.array_element);
				else
					printf("%s: unable to link resource %s (binding:%d, type:uniform buffer)\n", filename.c_str(), link.resource_name.c_str(), link.binding);
			}
			break;
			case DescriptorType::storage_buffer:
			{
				auto pStorageBuffer = (UniformBuffer*)pResource->getBuffer(link.resource_name);
				if (pStorageBuffer)
					vk::descriptorPool.addWrite(m_descriptorSet, _vkDescriptorType(type), link.binding, &pStorageBuffer->m_info, link.array_element);
				else
					printf("%s: unable to link resource %s (binding:%d, type:storage buffer)\n", filename.c_str(), link.resource_name.c_str(), link.binding);
			}
			break;
			case DescriptorType::storage_image:
			{
				auto pStorageImage = pResource->getImage(link.resource_name);
				if (pStorageImage)
					vk::descriptorPool.addWrite(m_descriptorSet, _vkDescriptorType(type), link.binding, pStorageImage->getInfo(0), link.array_element);
				else
					printf("%s: unable to link resource %s (binding:%d, type:storage image)\n", filename.c_str(), link.resource_name.c_str(), link.binding);
			}
			break;
			case DescriptorType::image_n_sampler:
			{
				auto pTexture = pResource->getImage(link.resource_name);
				if (pTexture)
				{
					VkSampler sampler = 0;
					switch (link.sampler)
					{
					case SamplerType::none:
						break;
					case SamplerType::plain:
						sampler = vk::plainSampler;
						break;
					case SamplerType::plain_unnormalized:
						sampler = vk::plainUnnormalizedSampler;
						break;
					case SamplerType::color:
						sampler = vk::colorSampler;
						break;
					case SamplerType::color_border:
						sampler = vk::colorBorderSampler;
						break;
					}
					vk::descriptorPool.addWrite(m_descriptorSet, _vkDescriptorType(type), link.binding, pTexture->getInfo(sampler), link.array_element);
				}
				else
					printf("%s: unable to link resource %s (binding:%d, type:combined image sampler)\n", filename.c_str(), link.resource_name.c_str(), link.binding);
			}
			break;
			}
		}

		vk::descriptorPool.update();
	}

	Drawcall::Drawcall() {}

	Drawcall::Drawcall(int vertexCount, int firstVertex, int instanceCount, int firstInstance)
	{
		vertex_count = vertexCount;
		first_vertex = firstVertex;
		instance_count = instanceCount;
		first_instance = firstInstance;
		type = DrawcallType::vertex;
	}

	Drawcall::Drawcall(int indexCount, int firstIndex, int vertexOffset, int instanceCount, int firstInstance)
	{
		index_count = indexCount;
		first_index = firstIndex;
		vertex_offset = vertexOffset;
		instance_count = instanceCount;
		first_instance = firstInstance;
		type = DrawcallType::index;
	}

	Drawcall::Drawcall(Model *p, int instanceCount, int firstInstance)
	{
		index_count = p->indices.size();
		first_index = p->indiceBase;
		vertex_offset = p->vertexBase;
		instance_count = instanceCount;
		first_instance = firstInstance;
		type = DrawcallType::index;
	}

	Drawcall::Drawcall(IndirectVertexBuffer *indirectVertexBuffer, uint32_t firstIndirect, uint32_t indirectCount)
	{
		m_indirectVertexBuffer = indirectVertexBuffer;
		first_indirect = firstIndirect;
		indirect_count = indirectCount;
		type = DrawcallType::indirect_vertex;
	}

	Drawcall::Drawcall(IndirectIndexBuffer *indirectIndexBuffer, uint32_t firstIndirect, uint32_t indirectCount)
	{
		m_indirectIndexBuffer = indirectIndexBuffer;
		first_indirect = firstIndirect;
		indirect_count = indirectCount;
		type = DrawcallType::indirect_index;
	}

	Drawcall::Drawcall(StageType stage, void *data, size_t size, size_t offset)
	{
		push_constant_stage = stage;
		push_constant_offset = offset;
		push_constant_size = size;
		push_constant_value = malloc(size);
		memcpy(push_constant_value, data, size);
		type = DrawcallType::push_constant;
	}

	Drawcall::~Drawcall()
	{
		delete push_constant_value;
	}

	void Drawcall::loadFromAt(AttributeTreeNode *n)
	{
		n->obtainFromAttributes(this, b);
	}

	void Drawcall::saveToAt(AttributeTreeNode *n)
	{
		n->addAttributes(this, b);
	}

	Drawcall *DrawAction::findDrawcall(const std::string &n)
	{
		for (auto &c : drawcalls)
			if (c.name == n)
				return &c;
		return nullptr;
	}

	DrawAction::DrawAction() {}

	DrawAction::DrawAction(Pipeline *pipeline)
	{
		m_pipeline = pipeline;
		type = DrawActionType::draw_action;
	}

	DrawAction::DrawAction(PF_RenderFunc pRenderFunc)
	{
		m_pRenderFunc = pRenderFunc;
		type = DrawActionType::call_fuction;
	}

	void DrawAction::loadFromAt(AttributeTreeNode *n)
	{
		n->obtainFromAttributes(this, b);

		auto drawcallsNode = n->firstNode("drawcalls");
		if (drawcallsNode)
		{
			for (auto nn : drawcallsNode->children)
			{
				if (nn->name == "drawcall")
				{
					auto d = addDrawcall();
					d->loadFromAt(nn);
				}
			}
		}
	}

	void DrawAction::saveToAt(AttributeTreeNode *n)
	{
		n->addAttributes(this, b);

		auto drawcallsNode = new AttributeTreeNode("drawcalls");
		n->children.push_back(drawcallsNode);
		for (auto &d : drawcalls)
		{
			auto n = new AttributeTreeNode("drawcall");
			drawcallsNode->children.push_back(n);
			d.saveToAt(n);
		}
	}

	void DrawAction::maintain(int row)
	{
		maintainList(drawcalls);
	}

	void DrawAction::preprocess(Pipeline* &currentPipeline)
	{
		if (m_pipeline)
		{
			if (!m_descriptorSet)
			{
				if (m_pipeline->m_descriptorSet)
					m_descriptorSet = m_pipeline->m_descriptorSet;
			}
		}

		if (m_pipeline && m_pipeline != currentPipeline)
			currentPipeline = m_pipeline;
	}

	Attachment::Attachment() {}

	Attachment::Attachment(Image *_image, VkClearValue _clearValue)
	{
		image = _image;
		if (_clearValue.color.float32[0] == 9999.f) return;
		loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		clearValue = _clearValue;
	}

	void Attachment::loadFromAt(AttributeTreeNode *n)
	{
		n->obtainFromAttributes(this, b);
	}

	void Attachment::saveToAt(AttributeTreeNode *n)
	{
		n->addAttributes(this, b);
	}

	Dependency::Dependency() {}

	Dependency::Dependency(void *_target)
	{
		target = _target;
	}

	Dependency::Dependency(const std::string &_pass_name)
	{
		pass_name = _pass_name;
	}

	void Dependency::loadFromAt(AttributeTreeNode *n)
	{
		n->obtainFromAttributes(this, b);
	}

	void Dependency::saveToAt(AttributeTreeNode *n)
	{
		n->addAttributes(this, b);
	}

	Attachment *RenderPass::findAttachment(const std::string &n)
	{
		for (auto &a : attachments)
			if (a.name == n)
				return &a;
		return nullptr;
	}

	DrawAction *RenderPass::findAction(const std::string &n)
	{
		for (auto &a : actions)
			if (a.name == n)
				return &a;
		return nullptr;
	}

	RenderPass::RenderPass() {}

	RenderPass::RenderPass(VkCommandBuffer cmd)
	{
		type = RenderPassType::call_secondary_cmd;
		secondaryCmd = cmd;
	}

	void RenderPass::loadFromAt(AttributeTreeNode *n)
	{
		n->obtainFromAttributes(this, b);

		auto attachmentNode = n->firstNode("attachments");
		if (attachmentNode)
		{
			for (auto n : attachmentNode->children)
			{
				if (n->name == "attachment")
				{
					auto attachment = addAttachment();
					attachment->loadFromAt(n);
				}
			}
		}
		auto dependenciesNode = n->firstNode("dependencies");
		if (dependenciesNode)
		{
			for (auto n : dependenciesNode->children)
			{
				if (n->name == "dependency")
				{
					auto dependency = addDependency();
					dependency->loadFromAt(n);
				}
			}
		}
		auto actionsNode = n->firstNode("actions");
		if (actionsNode)
		{
			for (auto n : actionsNode->children)
			{
				if (n->name == "action")
				{
					auto action = addAction();
					action->loadFromAt(n);
				}
			}
		}
	}

	void RenderPass::saveToAt(AttributeTreeNode *n)
	{
		n->addAttributes(this, b);

		auto attachmentNode = new AttributeTreeNode("attachments");
		n->children.push_back(attachmentNode);
		for (auto &a : attachments)
		{
			auto n = new AttributeTreeNode("attachment");
			attachmentNode->children.push_back(n);
			a.saveToAt(n);
		}
		auto dependenciesNode = new AttributeTreeNode("dependencies");
		n->children.push_back(dependenciesNode);
		for (auto &d : dependencies)
		{
			auto n = new AttributeTreeNode("dependency");
			dependenciesNode->children.push_back(n);
			d.saveToAt(n);
		}
		auto actionsNode = new AttributeTreeNode("actions");
		n->children.push_back(actionsNode);
		for (auto &a : actions)
		{
			auto n = new AttributeTreeNode("action");
			actionsNode->children.push_back(n);
			a.saveToAt(n);
		}
	}

	void RenderPass::maintain(int row)
	{
		switch (row)
		{
		case (int)RenderPassElement::eAction:
			maintainList(actions);
			break;
		case (int)RenderPassElement::eAttachment:
			maintainList(attachments);
			break;
		case (int)RenderPassElement::eDependency:
			maintainList(dependencies);
			break;
		}
	}

	RenderPass *Renderer::findRenderPass(const std::string &n)
	{
		for (auto &p : passes)
			if (p.name == n)
				return &p;
		return nullptr;
	}

	Renderer::Renderer() {}

	Renderer::Renderer(int _cx, int _cy)
	{
		cx = _cx;
		cy = _cy;
	}

	Renderer::~Renderer()
	{
	}

	void Renderer::loadXML(const std::string &_filename)
	{
		filename = _filename;

		AttributeTree at("renderer");
		at.loadXML(filename);

		at.obtainFromAttributes(this, b);

		auto buffersNode = at.firstNode("buffers");
		if (buffersNode)
		{
			for (auto c : buffersNode->children)
			{
				if (c->name == "buffer")
				{
					BufferResource b;
					c->obtainFromAttributes(&b, b.b);
					bufferResources.push_back(b);
				}
			}
		}

		auto imagesNode = at.firstNode("images");
		if (imagesNode)
		{
			for (auto c : imagesNode->children)
			{
				if (c->name == "image")
				{
					ImageResource i;
					c->obtainFromAttributes(&i, i.b);
					imageResources.push_back(i);
				}
			}
		}

		auto pipelinesNode = at.firstNode("pipelines");
		if (pipelinesNode)
		{
			for (auto c : pipelinesNode->children)
			{
				if (c->name == "pipeline")
				{
					PipelineResource p;
					c->obtainFromAttributes(&p, p.b);
					pipelineResources.push_back(p);
				}
			}
		}

		auto passesNode = at.firstNode("passes");
		if (passesNode)
		{
			for (auto c : passesNode->children)
			{
				if (c->name == "pass")
				{
					auto pass = addPass();
					pass->loadFromAt(c);
				}
			}
		}

		for (auto &p : passes)
		{
			for (auto &d : p.dependencies)
			{
				for (auto &pp : passes)
				{
					if (d.pass_name == pp.name)
					{
						d.target = &pp;
						break;
					}
				}
			}
		}
	}

	void Renderer::saveXML()
	{
		for (auto &p : passes)
		{
			for (auto &d : p.dependencies)
			{
				if (d.target)
				{
					auto pp = (RenderPass*)d.target;
					d.pass_name = pp->name;
				}
			}
		}

		AttributeTree at("renderer");

		at.addAttributes(this, b);

		auto passesNode = new AttributeTreeNode("passes");
		at.children.push_back(passesNode);
		for (auto &p : passes)
		{
			auto n = new AttributeTreeNode("pass");
			passesNode->children.push_back(n);
			p.saveToAt(n);
		}

		at.saveXML(filename);
	}

	void Renderer::maintain(int row)
	{
		maintainList(passes);
	}

	void Renderer::pushImage(Attachment *ai)
	{
		auto view = ai->image->getView(VkImageAspectFlags(ai->aspect), ai->level, 1, ai->layer, 1);

		auto index = 0;
		for (; index < vkViews[0].size(); index++)
		{
			if (vkViews[0][index] == view)
			{
				ai->index = index;
				return;
			}
		}
		VkAttachmentDescription attachment;
		switch (ai->image->type)
		{
		case Image::eColor:
			attachment = vk::colorAttachment(ai->image->m_format, ai->loadOp);
			break;
		case Image::eSwapchain:
			attachment = vk::swapchainAttachment(ai->loadOp);
			break;
		case Image::eDepth:
			attachment = vk::depthAttachment(ai->image->m_format, ai->loadOp);
			break;
		case Image::eDepthStencil:
			assert(0);
			//attachment = Vk::depthAttachment(ai->image->m_format, ai->loadOp);
			break;
		}
		vkAttachments.push_back(attachment);
		vkViews[0].push_back(view);
		if (containSwapchain)
		{
			if (ai->image->type == Image::eSwapchain)
				view = ai->image[1].getView(VkImageAspectFlags(ai->aspect), ai->layer, 1, ai->layer, 1);
			vkViews[1].push_back(view);
		}
		vkClearValues.push_back(ai->clearValue);
		ai->index = index;
	}

	void Renderer::getDescriptorSets()
	{
		currentPipeline = initPipeline;

		for (auto &pass : passes)
		{
			for (auto &action : pass.actions)
				action.preprocess(currentPipeline);
		}

		currentPipeline = nullptr;
	}

	void Renderer::execute(VkCommandBuffer cmd, int index)
	{
		currentVertexBuffer = initVertexBuffer;
		currentIndexBuffer = initIndexBuffer;
		currentPipeline = initPipeline;
		currentDescriptorSet = initDescriptorSet;

		vkCmdBeginRenderPass(cmd, &vk::renderPassBeginInfo(vkRenderPass, vkFramebuffer[index], cx, cy, vkClearValues.size(), vkClearValues.data()), VK_SUBPASS_CONTENTS_INLINE);

		if (currentVertexBuffer)
			currentVertexBuffer->bind(cmd);

		if (currentIndexBuffer)
			currentIndexBuffer->bind(cmd);

		bool firstPass = true;
		for (auto &pass : passes)
		{
			switch (pass.type)
			{
			case RenderPassType::draw_action:
				if (!firstPass) vkCmdNextSubpass(cmd, VK_SUBPASS_CONTENTS_INLINE);
				for (auto &action : pass.actions)
				{
					if (!action.show) continue;

					if (action.cx || action.cy)
					{
						VkViewport viewport;
						viewport.x = 0;
						viewport.y = 0;
						viewport.width = action.cx;
						viewport.height = action.cy;
						viewport.minDepth = 0.0f;
						viewport.maxDepth = 1.0f;
						vkCmdSetViewport(cmd, 0, 1, &viewport);

						VkRect2D scissor;
						scissor.offset.x = 0;
						scissor.offset.y = 0;
						scissor.extent.width = action.cx;
						scissor.extent.height = action.cy;
						vkCmdSetScissor(cmd, 0, 1, &scissor);
					}
					switch (action.type)
					{
					case DrawActionType::draw_action:
						if (action.m_pipeline && action.m_pipeline != currentPipeline)
						{
							vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, action.m_pipeline->m_pipeline);
							currentPipeline = action.m_pipeline;
						}
						if (action.m_vertexBuffer && action.m_vertexBuffer != currentVertexBuffer)
						{
							action.m_vertexBuffer->bind(cmd);
							currentVertexBuffer = action.m_vertexBuffer;
						}
						if (action.m_indexBuffer && action.m_indexBuffer != currentIndexBuffer)
						{
							action.m_indexBuffer->bind(cmd);
							currentIndexBuffer = action.m_indexBuffer;
						}
						if (action.m_descriptorSet && action.m_descriptorSet != currentDescriptorSet)
						{
							vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, action.m_pipeline->m_pipelineLayout, 0, 1, &action.m_descriptorSet, 0, nullptr);
							currentDescriptorSet = action.m_descriptorSet;
						}

						for (auto &drawcall : action.drawcalls)
						{
							switch (drawcall.type)
							{
							case DrawcallType::vertex:
								vkCmdDraw(cmd, drawcall.vertex_count, drawcall.instance_count, drawcall.first_vertex, drawcall.first_instance);
								break;
							case DrawcallType::index:
								vkCmdDrawIndexed(cmd, drawcall.index_count, drawcall.instance_count, drawcall.first_index, drawcall.vertex_offset, drawcall.first_instance);
								break;
							case DrawcallType::indirect_vertex:
								vkCmdDrawIndirect(cmd, drawcall.m_indirectVertexBuffer->m_buffer, drawcall.first_indirect * drawcall.m_indirectVertexBuffer->stride(), drawcall.indirect_count, drawcall.m_indirectVertexBuffer->stride());
								break;
							case DrawcallType::indirect_index:
								vkCmdDrawIndexedIndirect(cmd, drawcall.m_indirectIndexBuffer->m_buffer, drawcall.first_indirect * drawcall.m_indirectIndexBuffer->stride(), drawcall.indirect_count, drawcall.m_indirectIndexBuffer->stride());
								break;
							case DrawcallType::push_constant:
								vkCmdPushConstants(cmd, currentPipeline->m_pipelineLayout, vkStage(drawcall.push_constant_stage), drawcall.push_constant_offset, drawcall.push_constant_size, drawcall.push_constant_value);
								break;
							}
						}
						break;
					case DrawActionType::call_fuction:
						if (action.m_pRenderFunc)
							action.m_pRenderFunc(cmd);
						break;
					}
				}
				break;
			case RenderPassType::call_secondary_cmd:
				if (!firstPass) vkCmdNextSubpass(cmd, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
				if (pass.secondaryCmd) vkCmdExecuteCommands(cmd, 1, &pass.secondaryCmd);
				break;
			}
			firstPass = false;
		}

		vkCmdEndRenderPass(cmd);

		currentVertexBuffer = nullptr;
		currentIndexBuffer = nullptr;
		currentPipeline = nullptr;
		currentDescriptorSet = 0;
	}

	void Renderer::setup()
	{
		if (cx == -1)
			cx = tke::resCx;
		if (cy == -1)
			cy = tke::resCy;

		for (auto &b : bufferResources)
		{
			b.p = new UniformBuffer;
			b.p->create(b.size);
			resource.setBuffer(b.p, b.name);
		}

		for (auto &i : imageResources)
		{
			if (i.cx == -1)
				i.cx = tke::resCx;
			if (i.cy == -1)
				i.cy = tke::resCy;

			if (i.file_name != "")
			{
				i.p = createImage(enginePath + i.file_name, i.sRGB);
			}
			else
			{
				i.p = new Image;
				i.p->create(i.cx, i.cy, vkFormat(i.format), VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
			}
			resource.setImage(i.p, i.name);
		}

		for (auto &p : pipelineResources)
		{
			p.p = new Pipeline;
			p.p->pResource = &resource;
			resource.setPipeline(p.p, p.name);
		}

		if (vertex_buffer_name!= "")
			initVertexBuffer = (VertexBuffer*)resource.getBuffer(vertex_buffer_name);
		if (index_buffer_name != "")
			initIndexBuffer = (IndexBuffer*)resource.getBuffer(index_buffer_name);

		for (auto &p : passes)
		{
			for (auto &a : p.attachments)
			{
				if (a.image_name != "")
					a.image = resource.getImage(a.image_name);
				if (a.image->type == Image::eSwapchain)
					containSwapchain = true;
				if (a.clear)
				{
					a.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
					if (a.image->isColorType())
					{
						a.clearValue.color.float32[0] = a.clear_r;
						a.clearValue.color.float32[1] = a.clear_g;
						a.clearValue.color.float32[2] = a.clear_b;
						a.clearValue.color.float32[3] = a.clear_a;
					}
					else
					{
						a.clearValue.depthStencil.depth = a.clear_depth;
						a.clearValue.depthStencil.stencil = a.clear_stencil;
					}
				}
			}
		}

		vkAttachments.clear();
		vkViews[0].clear();
		vkViews[1].clear();
		std::vector<VkSubpassDescription> vkSubpasses;
		std::vector<VkSubpassDependency> vkDependencies;

		std::vector<std::vector<VkAttachmentReference>> vkRefLists(passes.size());

		int subpassIndex = 0;
		for (auto &pass : passes)
		{
			pass.index = subpassIndex;

			for (auto &a : pass.attachments)
				pushImage(&a);

			VkSubpassDescription desc = {};
			desc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

			bool hasDepthStencil = false;
			for (auto &a : pass.attachments)
			{
				if (a.image->isColorType())
				{
					vkRefLists[subpassIndex].push_back({ (unsigned int)a.index, VK_IMAGE_LAYOUT_GENERAL });
					desc.colorAttachmentCount++;
				}
				else
				{
					vkRefLists[subpassIndex].insert(vkRefLists[subpassIndex].begin(), { (unsigned int)a.index, VK_IMAGE_LAYOUT_GENERAL });
					hasDepthStencil = true;
				}
			}

			if (desc.colorAttachmentCount > 0)
				desc.pColorAttachments = hasDepthStencil ? &vkRefLists[subpassIndex][1] : vkRefLists[subpassIndex].data();
			if (hasDepthStencil)
				desc.pDepthStencilAttachment = vkRefLists[subpassIndex].data();

			vkSubpasses.push_back(desc);

			for (auto &dependency : pass.dependencies)
			{
				auto pass = (RenderPass*)dependency.target;
				vkDependencies.push_back(vk::subpassDependency(pass->index, subpassIndex));
			}

			switch (pass.type)
			{
			case RenderPassType::draw_action:
				for (auto &action : pass.actions)
				{
					switch (action.type)
					{
					case DrawActionType::draw_action:
						if (action.pipeline_name != "")
						{
							action.m_pipeline = resource.getPipeline(action.pipeline_name.c_str());
							if (action.m_pipeline)
							{
								for (auto &p : pipelineResources)
								{
									if (p.name == action.pipeline_name)
										p.subpassIndex = subpassIndex;
								}
							}
						}
						for (auto &drawcall : action.drawcalls)
						{
							if (drawcall.indirect_vertex_buffer_name != "")
								drawcall.m_indirectVertexBuffer = (IndirectVertexBuffer*)resource.getBuffer(drawcall.indirect_vertex_buffer_name);
							if (drawcall.indirect_index_buffer_name != "")
								drawcall.m_indirectIndexBuffer = (IndirectIndexBuffer*)resource.getBuffer(drawcall.indirect_index_buffer_name);
							if (drawcall.model_name != "")
							{
								auto p = resource.getModel(drawcall.model_name);
								if (p)
								{
									drawcall.index_count = p->indices.size();
									drawcall.first_index = p->indiceBase;
									drawcall.vertex_offset = p->vertexBase;
								}
							}
						}
						break;
					case DrawActionType::call_fuction:
						break;
					}
				}
				break;
			case RenderPassType::call_secondary_cmd:
				if (pass.secondary_cmd_name != "")
					pass.secondaryCmd = resource.getCmd(pass.secondary_cmd_name.c_str());
				break;
			}

			subpassIndex++;
		}

		if (vkRenderPass) vk::destroyRenderPass(vkRenderPass);
		vkRenderPass = vk::createRenderPass(vkAttachments.size(), vkAttachments.data(),
			vkSubpasses.size(), vkSubpasses.data(), vkDependencies.size(), vkDependencies.data());

		if (vkFramebuffer[0]) destroyFramebuffer(vkFramebuffer[0]);
		vkFramebuffer[0] = createFramebuffer(cx, cy, vkRenderPass, vkViews[0]);
		if (containSwapchain)
		{
			if (vkFramebuffer[1]) destroyFramebuffer(vkFramebuffer[1]);
			vkFramebuffer[1] = createFramebuffer(cx, cy, vkRenderPass, vkViews[1]);
		}

		for (auto &p : pipelineResources)
			p.p->create(enginePath + p.file_name, p.vertex_input_type == VertexInputType::zero ? &zeroVertexInputState : &vertexInputState, vkRenderPass, p.subpassIndex);

		getDescriptorSets();
	}
}
