#include <experimental/filesystem>
#include <regex>

#include "core.h"
#include "gui.h"
#include "render.h"
#include "model.h"

namespace tke
{
	void Buffer::create(size_t size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memoryProperty)
	{
		m_size = size;
		vk::createBuffer(size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage, memoryProperty, m_buffer, m_memory);
	}

	void Buffer::destory()
	{
		if (m_buffer && m_memory)
			vk::destroyBuffer(m_buffer, m_memory);
	}

	void StagingBuffer::create(size_t size)
	{
		Buffer::create(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	}

	void NonStagingBufferAbstract::create(size_t size, VkBufferUsageFlags usage, void *data)
	{
		Buffer::create(size, usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		if (data)
		{
			StagingBuffer stagingBuffer;
			stagingBuffer.create(size);

			void* map = vk::mapMemory(stagingBuffer.m_memory, 0, size);
			memcpy(map, data, size);
			vk::unmapMemory(stagingBuffer.m_memory);
			vk::copyBuffer(stagingBuffer.m_buffer, m_buffer, size);

			stagingBuffer.destory();
		}
	}

	void NonStagingBufferAbstract::update(void *data, StagingBuffer *pStagingBuffer, size_t size)
	{
		if (size == 0) size = m_size;
		vk::updateBuffer(data, size, pStagingBuffer->m_buffer, pStagingBuffer->m_memory, m_buffer);
	}

	void ShaderManipulatableBufferAbstract::create(size_t size, VkBufferUsageFlags usage)
	{
		NonStagingBufferAbstract::create(size, usage);
		m_info = {};
		m_info.buffer = m_buffer;
		m_info.range = m_size;
	}

	void UniformBuffer::create(size_t size)
	{
		ShaderManipulatableBufferAbstract::create(size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
	}

	void VertexBuffer::create(size_t size, void *data)
	{
		NonStagingBufferAbstract::create(size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, data);
	}

	void VertexBuffer::bind(VkCommandBuffer cmd)
	{
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(cmd, 0, 1, &m_buffer, offsets);
	}

	void IndexBuffer::create(size_t size, void *data)
	{
		NonStagingBufferAbstract::create(size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, data);
	}

	void IndexBuffer::bind(VkCommandBuffer cmd)
	{
		vkCmdBindIndexBuffer(cmd, m_buffer, 0, VK_INDEX_TYPE_UINT32);
	}

	size_t VertexIndirectBuffer::stride()
	{
		return sizeof(VkDrawIndirectCommand);
	}

	void VertexIndirectBuffer::create(size_t size)
	{
		NonStagingBufferAbstract::create(size, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT);
	}

	size_t IndexedIndirectBuffer::stride()
	{
		return sizeof(VkDrawIndexedIndirectCommand);
	}

	void IndexedIndirectBuffer::create(size_t size)
	{
		NonStagingBufferAbstract::create(size, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT);
	}

	Image::~Image()
	{
		if (m_image && m_memory)
			vk::destroyImage(m_image, m_memory);
	}

	void Image::transitionLayout(int level, VkImageAspectFlags aspect, VkImageLayout layout)
	{
		vk::transitionImageLayout(m_image, aspect, m_layout, layout, level);
		m_layout = layout;
	}

	void Image::fillData(int level, std::uint8_t *data, size_t size, VkImageAspectFlags aspect)
	{
		transitionLayout(level, aspect, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingMemory;

		vk::createBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingMemory);

		void* map = vk::mapMemory(stagingMemory, 0, size);
		memcpy(map, data, size);
		vk::unmapMemory(stagingMemory);

		VkBufferImageCopy region = {};
		region.imageSubresource.aspectMask = aspect;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;
		region.imageExtent.width = getWidth(level);
		region.imageExtent.height = getHeight(level);
		region.imageExtent.depth = 1;
		region.bufferOffset = 0;

		auto cmd = vk::commandPool.begineOnce();
		vkCmdCopyBufferToImage(cmd, stagingBuffer, m_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
		vk::commandPool.endOnce(cmd);

		vk::destroyBuffer(stagingBuffer, stagingMemory);
	}

	void Image::create(int w, int h, VkFormat format, VkImageUsageFlags usage, std::uint8_t *data, size_t size, VkImageAspectFlags aspect)
	{
		m_width = w;
		m_height = h;
		m_format = format;
		if (format == VK_FORMAT_D16_UNORM || format == VK_FORMAT_D32_SFLOAT)
			type = eDepth;
		else if (format == VK_FORMAT_D16_UNORM_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT || format == VK_FORMAT_D32_SFLOAT_S8_UINT)
			type = eDepthStencil;
		else
			type = eColor;

		if (aspect == 0)
		{
			if (type == eColor || type == eSwapchain)
				aspect = VK_IMAGE_ASPECT_COLOR_BIT;
			else
				aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
		}

		m_size = vk::createImage(w, h, m_mipmapLevels, m_arrayLayers, format, usage, m_image, m_memory);

		if (data && size)
		{
			assert(size <= m_size);
			fillData(0, data, size, aspect);
		}
		transitionLayout(0, aspect, VK_IMAGE_LAYOUT_GENERAL);
	}

	VkImageView Image::getView(VkImageAspectFlags aspect, int baseLevel, int levelCount, int baseLayer, int layerCount)
	{
		if (aspect == 0)
		{
			if (type == eColor || type == eSwapchain)
				aspect = VK_IMAGE_ASPECT_COLOR_BIT;
			else
				aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
		}

		for (auto &view : views)
		{
			if (view.aspect == aspect && view.baseLevel == baseLevel && view.levelCount == levelCount &&
				view.baseLayer == baseLayer && view.layerCount == layerCount)
				return view.view;
		}
		View view;
		view.aspect = aspect;
		view.baseLevel = baseLevel;
		view.levelCount = levelCount;
		view.baseLayer = baseLayer;
		view.layerCount = layerCount;
		view.view = vk::createImageView(m_image, m_viewType, aspect, m_format, baseLevel, levelCount, baseLayer, layerCount);
		views.push_back(view);
		return view.view;
	}

	VkDescriptorImageInfo *Image::getInfo(VkSampler sampler, VkImageAspectFlags aspect, int baseLevel, int levelCount, int baseLayer, int layerCount)
	{
		auto view = getView(aspect, baseLevel, levelCount, baseLayer, layerCount);
		for (auto info : infos)
		{
			if (info->imageView == view && info->sampler == sampler)
				return info;
		}
		auto info = new VkDescriptorImageInfo;
		info->imageView = view;
		info->sampler = sampler;
		info->imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		infos.push_back(info);
		return info;
	}

	int Image::getWidth(int mipmapLevel) const
	{
		int w = m_width;
		for (;;)
		{
			if (mipmapLevel <= 0 || w < 2)
				break;
			mipmapLevel--;
			w >>= 1;
		}
		return w;
	}

	int Image::getHeight(int mipmapLevel) const
	{
		int h = m_height;
		for (;;)
		{
			if (mipmapLevel <= 0 || h < 2)
				break;
			mipmapLevel--;
			h >>= 1;
		}
		return h;
	}

	unsigned char Image::getPixel(int x, int y, int off) const
	{
		//if (!m_data || x >= m_width || y >= m_height)
		//	return 0;

		//int pixelLength = 4;
		//int lineLength = PITCH(m_width);
		//lineLength = PITCH(m_width * pixelLength);

		//return m_data[x * pixelLength + y * lineLength + off];

		return 0;
	}

	std::vector<DescriptorSetLayout> descriptorSetLayouts;
	VkDescriptorSetLayout getDescriptorSetLayout(std::vector<VkDescriptorSetLayoutBinding> &bindings)
	{
		for (auto &desc : descriptorSetLayouts)
		{
			if (desc.bindings.size() == bindings.size())
			{
				bool same = true;
				for (auto i = 0; i < desc.bindings.size(); i++)
				{
					auto &binding = desc.bindings[i];
					if (binding.binding != bindings[i].binding || binding.descriptorCount != bindings[i].descriptorCount ||
						binding.descriptorType != bindings[i].descriptorType || binding.stageFlags != bindings[i].stageFlags)
					{
						same = false;
						break;
					}
				}
				if (same)
					return desc.layout;
			}
		}
		DescriptorSetLayout descriptorSetLayout;
		descriptorSetLayout.bindings.insert(descriptorSetLayout.bindings.begin(), bindings.begin(), bindings.end());
		VkDescriptorSetLayoutCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		info.bindingCount = bindings.size();
		info.pBindings = bindings.data();
		descriptorSetLayout.layout = vk::createDescriptorSetLayout(&info);
		descriptorSetLayouts.push_back(descriptorSetLayout);
		return descriptorSetLayout.layout;
	}

	std::vector<PipelineLayout> pipelineLayouts;
	VkPipelineLayout getPipelineLayout(VkDescriptorSetLayout descriptorLayout, std::vector<VkPushConstantRange> &pushConstantRanges)
	{
		for (auto &pipe : pipelineLayouts)
		{
			if (pipe.descriptorLayout == descriptorLayout && pipe.pushConstantRanges.size() == pushConstantRanges.size())
			{
				bool same = true;
				for (auto i = 0; i < pipe.pushConstantRanges.size(); i++)
				{
					auto &pc = pipe.pushConstantRanges[i];
					if (pc.offset != pushConstantRanges[i].offset || pc.size != pushConstantRanges[i].size ||
						pc.stageFlags != pushConstantRanges[i].stageFlags)
					{
						same = false;
						break;
					}
				}
				if (same)
					return pipe.layout;
			}
		}
		PipelineLayout pipelineLayout;
		pipelineLayout.descriptorLayout = descriptorLayout;
		pipelineLayout.pushConstantRanges.insert(pipelineLayout.pushConstantRanges.begin(), pushConstantRanges.begin(), pushConstantRanges.end());
		VkPipelineLayoutCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		info.setLayoutCount = 1;
		info.pSetLayouts = &pipelineLayout.descriptorLayout;
		info.pushConstantRangeCount = pipelineLayout.pushConstantRanges.size();
		info.pPushConstantRanges = pipelineLayout.pushConstantRanges.data();
		pipelineLayout.layout = vk::createPipelineLayout(&info);
		pipelineLayouts.push_back(pipelineLayout);
		return pipelineLayout.layout;
	}

	std::vector<Framebuffer> framebuffers;
	VkFramebuffer getFramebuffer(int cx, int cy, VkRenderPass renderPass, std::vector<VkImageView> &views)
	{
		for (auto &framebuffer : framebuffers)
		{
			if (framebuffer.views.size() == views.size())
			{
				bool same = true;
				for (auto i = 0; i < framebuffer.views.size(); i++)
				{
					if (framebuffer.views[i] != views[i])
					{
						same = false;
						break;
					}
				}
				if (same)
					return framebuffer.framebuffer;
			}
		}
		Framebuffer framebuffer;
		framebuffer.views.insert(framebuffer.views.begin(), views.begin(), views.end());
		framebuffer.framebuffer = vk::createFramebuffer(cx, cy, renderPass, views.size(), views.data());
		framebuffers.push_back(framebuffer);
		return framebuffer.framebuffer;
	}

	std::vector<ShaderModule> shaderModules;
	VkShaderModule getShaderModule(const std::string &filename)
	{
		// format the shader path, so that they can reuse if them refer the same one
		auto path = std::experimental::filesystem::canonical(filename);
		for (auto &shader : shaderModules)
		{
			if (shader.filename == path.string())
				return shader.module;
		}
		ShaderModule shaderModule;
		shaderModule.filename = path.string();
		shaderModule.module = vk::loadShaderModule(shaderModule.filename);
		shaderModules.push_back(shaderModule);
		return shaderModule.module;
	}

	VkBlendFactor _vkBlendFactor(BlendFactor f) 
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
	VkDescriptorType _vkDescriptorType(DescriptorType t)
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
	VkShaderStageFlags _vkStage(StageFlags f)
	{
		VkShaderStageFlags v = 0;
		if ((int)f & (int)StageFlags::vert) v |= VK_SHADER_STAGE_VERTEX_BIT;
		if ((int)f & (int)StageFlags::tesc) v |= VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
		if ((int)f & (int)StageFlags::tese) v |= VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
		if ((int)f & (int)StageFlags::geom) v |= VK_SHADER_STAGE_GEOMETRY_BIT;
		if ((int)f & (int)StageFlags::frag) v |= VK_SHADER_STAGE_FRAGMENT_BIT;
		return v;
	}

	int Pipeline::descriptorPosition(const std::string &name)
	{
		for (auto s : stages)
		{
			for (auto &d : s->descriptors)
			{
				if (d.name == name)
					return d.binding;
			}
		}
		assert(false);
		return -1;
	}

	void Pipeline::create(const char *_f, VkPipelineVertexInputStateCreateInfo *pVertexInputState, VkRenderPass renderPass, std::uint32_t subpassIndex)
	{
		setFilename(_f);

		m_pVertexInputState = pVertexInputState;

		m_renderPass = renderPass;
		m_subpassIndex = subpassIndex;

		PipelineAbstract::loadXML();

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
			VkPipelineShaderStageCreateInfo i = {};
			i.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			i.pName = "main";
			switch (s->type)
			{
			case StageFlags::vert: i.stage = VK_SHADER_STAGE_VERTEX_BIT; break;
			case StageFlags::tesc: i.stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT; break;
			case StageFlags::tese: i.stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT; break;
			case StageFlags::geom: i.stage = VK_SHADER_STAGE_GEOMETRY_BIT; break;
			case StageFlags::frag: i.stage = VK_SHADER_STAGE_FRAGMENT_BIT; break;
			}
			i.module = getShaderModule(filepath + "/" + s->filename + ".spv");
			vkStages.push_back(i);

			for (auto &d : s->descriptors)
			{
				auto found = false;
				for (auto &b : vkDescriptors)
				{
					if (b.binding == d.binding)
					{
						b.stageFlags |= _vkStage(s->type);
						found = true;
						break;
					}
				}
				if (found) continue;

				VkDescriptorSetLayoutBinding b = {};
				b.descriptorType = _vkDescriptorType(d.type);
				b.binding = d.binding;
				b.descriptorCount = d.count;
				b.stageFlags = _vkStage(s->type);
				vkDescriptors.push_back(b);
			}
			for (auto &p : s->pushConstantRanges)
			{
				auto found = false;
				for (auto &r : vkPushConstantRanges)
				{
					if (r.offset == p.offset & r.size == p.size)
					{
						r.stageFlags |= _vkStage(s->type);
						found = true;
						break;
					}
				}
				if (found) continue;

				VkPushConstantRange r = {};
				r.offset = p.offset;
				r.size = p.size;
				r.stageFlags = _vkStage(s->type);
				vkPushConstantRanges.push_back(r);
			}
		}

		m_descriptorSetLayout = getDescriptorSetLayout(vkDescriptors);
		m_pipelineLayout = getPipelineLayout(m_descriptorSetLayout, vkPushConstantRanges);

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

	Drawcall::Drawcall(VertexIndirectBuffer *vertexIndirectBuffer, uint32_t firstIndirect, uint32_t indirectCount)
	{
		m_vertexIndirectBuffer = vertexIndirectBuffer;
		first_indirect = firstIndirect;
		indirect_count = indirectCount;
		type = DrawcallType::indirect_vertex;
	}

	Drawcall::Drawcall(IndexedIndirectBuffer *indexedIndirectBuffer, uint32_t firstIndirect, uint32_t indirectCount)
	{
		m_indexedIndirectBuffer = indexedIndirectBuffer;
		first_indirect = firstIndirect;
		indirect_count = indirectCount;
		type = DrawcallType::indirect_index;
	}

	ImageResourceLink::ImageResourceLink(Image *image)
		: m_image(image) {}

	DrawAction::DrawAction() {}

	DrawAction::DrawAction(Pipeline *pipeline)
	{
		m_pipeline = pipeline;
		type = DrawActionType::draw_action;
	}

	DrawAction::DrawAction(Renderable *pRenderable)
	{
		m_pRenderable = pRenderable;
		type = DrawActionType::call_fuction;
	}

	void DrawAction::preprocess(Pipeline* &currentPipeline)
	{
		if (m_pipeline)
		{
			if (!m_descriptorSet)
			{
				if (m_pipeline->m_descriptorSet)
					m_descriptorSet = m_pipeline->m_descriptorSet;
				else if (m_imageResourceLinks.size() > 0)
					m_descriptorSet = vk::descriptorPool.allocate(&m_pipeline->m_descriptorSetLayout);
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

	Dependency::Dependency() {}

	Dependency::Dependency(void *_target)
	{
		target = _target;
	}

	Dependency::Dependency(const std::string &_pass_name)
	{
		pass_name = _pass_name;
	}

	RenderPass::RenderPass() {}

	RenderPass::RenderPass(VkCommandBuffer cmd)
	{
		type = RenderPassType::call_secondary_cmd;
		secondaryCmd = cmd;
	}

	Renderer::Renderer() {}

	Renderer::Renderer(int _cx, int _cy)
	{
		cx = _cx;
		cy = _cy;
	}


	void Renderer::loadXML()
	{
		RendererAbstract::loadXML();

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
								vkCmdDrawIndirect(cmd, drawcall.m_vertexIndirectBuffer->m_buffer, drawcall.first_indirect * drawcall.m_vertexIndirectBuffer->stride(), drawcall.indirect_count, drawcall.m_vertexIndirectBuffer->stride());
								break;
							case DrawcallType::indirect_index:
								vkCmdDrawIndexedIndirect(cmd, drawcall.m_indexedIndirectBuffer->m_buffer, drawcall.first_indirect * drawcall.m_indexedIndirectBuffer->stride(), drawcall.indirect_count, drawcall.m_indexedIndirectBuffer->stride());
								break;
							case DrawcallType::push_constant:
								vkCmdPushConstants(cmd, currentPipeline->m_pipelineLayout, drawcall.m_pushConstantStage, drawcall.push_constant_offset, drawcall.m_pushConstantSize, drawcall.push_constant_value);
								break;
							}
						}
						break;
					case DrawActionType::call_fuction:
						if (action.m_pRenderable)
							action.m_pRenderable->render(cmd);
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
		for (auto &p : passes)
		{
			for (auto &a : p.attachments)
			{
				if (a.image_name != "")
					a.image = pResource->getImage(a.image_name);
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
							action.m_pipeline = pResource->getPipeline(action.pipeline_name.c_str());
						break;
					case DrawActionType::call_fuction:
						break;
					}
				}
				break;
			case RenderPassType::call_secondary_cmd:
				if (pass.secondary_cmd_name != "")
					pass.secondaryCmd = pResource->getCmd(pass.secondary_cmd_name.c_str());
				break;
			}

			subpassIndex++;
		}

		if (vkRenderPass) vk::destroyRenderPass(vkRenderPass);
		vkRenderPass = vk::createRenderPass(vkAttachments.size(), vkAttachments.data(),
			vkSubpasses.size(), vkSubpasses.data(), vkDependencies.size(), vkDependencies.data());

		if (vkFramebuffer[0]) vk::destroyFramebuffer(vkFramebuffer[0]);
		vkFramebuffer[0] = getFramebuffer(cx, cy, vkRenderPass, vkViews[0]);
		if (containSwapchain)
		{
			if (vkFramebuffer[1]) vk::destroyFramebuffer(vkFramebuffer[1]);
			vkFramebuffer[1] = getFramebuffer(cx, cy, vkRenderPass, vkViews[1]);
		}
	}
}
