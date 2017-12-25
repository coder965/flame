#include "pipeline.h"
#include "descriptor.h"
#include "renderpass.h"
#include "shader.h"
#include "sampler.h"
#include "../core.h"
#include "../resource/resource.h"

namespace tke
{
	PipelineLayout::~PipelineLayout()
	{
		vkDestroyPipelineLayout(vk_device.v, v, nullptr);
	}

	static std::vector<std::weak_ptr<PipelineLayout>> pipelineLayouts;

	Pipeline::Pipeline(PipelineCreateInfo &info, RenderPass *_renderPass, int _subpassIndex, bool need_default_ds)
	{
		std::vector<std::vector<VkDescriptorSetLayoutBinding>> vkDescriptors;
		std::vector<VkPushConstantRange> vkPushConstantRanges;
		std::vector<VkPipelineShaderStageCreateInfo> vkStages;

		for (auto &sdesc : info._shaders)
		{
			std::shared_ptr<Shader> s;

			for (auto it = loaded_shaders.begin(); it != loaded_shaders.end(); )
			{
				s = it->lock();
				if (s)
				{
					if (s->filename == sdesc.first && s->defines.size() == sdesc.second.size())
					{
						bool same = true;
						for (int i = 0; i < sdesc.second.size(); i++)
						{
							if (s->defines[i] != sdesc.second[i])
							{
								same = false;
								break;
							}
						}

						if (same)
							goto next_shader;
					}

					it++;
				}
				else
					it = loaded_shaders.erase(it);
			}

			s = std::make_shared<Shader>(sdesc.first, sdesc.second);
			loaded_shaders.push_back(s);

		next_shader:

			{
				VkPipelineShaderStageCreateInfo info = {};
				info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
				info.pName = "main";
				info.stage = s->stage;
				info.module = s->vkModule;
				vkStages.push_back(info);

				if (vkDescriptors.size() < s->descriptors.size())
				{
					vkDescriptors.resize(s->descriptors.size());
					descriptorSetLayouts.resize(s->descriptors.size());
				}

				for (auto set = 0; set < s->descriptors.size(); set++)
				{
					for (auto &d : s->descriptors[set])
					{
						auto found = false;
						for (auto &b : vkDescriptors[set])
						{
							if (b.binding == d.binding)
							{
								b.stageFlags |= s->stage;
								found = true;
								break;
							}
						}
						if (found) continue;

						VkDescriptorSetLayoutBinding b = {};
						b.descriptorType = d.type;
						b.binding = d.binding;
						b.descriptorCount = d.count;
						b.stageFlags = s->stage;
						vkDescriptors[set].push_back(b);
					}
				}
				for (auto &p : s->pushConstantRanges)
				{
					auto found = false;
					for (auto &r : vkPushConstantRanges)
					{
						if (r.offset == p.offset & r.size == p.size)
						{
							r.stageFlags |= s->stage;
							found = true;
							break;
						}
					}
					if (found) continue;

					VkPushConstantRange r = {};
					r.offset = p.offset;
					r.size = p.size;
					r.stageFlags = s->stage;
					vkPushConstantRanges.push_back(r);
				}
			}

			shaders.push_back(s);
		}

		if (info._cx == -1)
			info._cx = resCx;
		if (info._cy == -1)
			info._cy = resCy;

		if (info._cx == 0 && info._cy == 0)
		{
			info._dynamicStates.push_back(VK_DYNAMIC_STATE_VIEWPORT);
			info._dynamicStates.push_back(VK_DYNAMIC_STATE_SCISSOR);
		}

		for (auto set = 0; set < vkDescriptors.size(); set++)
			descriptorSetLayouts[set] = getDescriptorSetLayout(vkDescriptors[set].size(), vkDescriptors[set].data());

		{
			bool found = false;
			for (auto it = pipelineLayouts.begin(); it != pipelineLayouts.end(); )
			{
				auto p = it->lock();

				if (p)
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

					it++;
				}
				else
					it = pipelineLayouts.erase(it);
			}

			if (!found)
			{
				auto p = std::make_shared <PipelineLayout>();
				for (auto d : descriptorSetLayouts)
					p->descriptorSetLayouts.push_back(d->v);
				p->pushConstantRanges.insert(p->pushConstantRanges.begin(), vkPushConstantRanges.begin(), vkPushConstantRanges.end());

				VkPipelineLayoutCreateInfo info = {};
				info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
				info.setLayoutCount = p->descriptorSetLayouts.size();
				info.pSetLayouts = p->descriptorSetLayouts.data();
				info.pushConstantRangeCount = p->pushConstantRanges.size();
				info.pPushConstantRanges = p->pushConstantRanges.data();

				auto res = vkCreatePipelineLayout(vk_device.v, &info, nullptr, &p->v);
				assert(res == VK_SUCCESS);

				pipelineLayouts.push_back(p);

				pipelineLayout = p;
			}
		}

		VkPipelineInputAssemblyStateCreateInfo assemblyState = {};
		assemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		assemblyState.topology = info._primitiveTopology;
		assemblyState.primitiveRestartEnable = VK_FALSE;

		VkPipelineTessellationStateCreateInfo tessState = {};
		tessState.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
		tessState.patchControlPoints = info._patch_control_points;

		VkPipelineDepthStencilStateCreateInfo depthStencilState = {};
		depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencilState.depthTestEnable = info._depth_test;
		depthStencilState.depthWriteEnable = info._depth_write;
		depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS;

		VkViewport viewport;
		viewport.width = (float)info._cx;
		viewport.height = (float)info._cy;
		viewport.minDepth = (float)0.0f;
		viewport.maxDepth = (float)1.0f;
		viewport.x = 0;
		viewport.y = 0;

		VkRect2D scissor;
		scissor.extent.width = info._cx;
		scissor.extent.height = info._cy;
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
		rasterState.polygonMode = info._polygonMode;
		rasterState.cullMode = info._cullMode;
		rasterState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterState.depthClampEnable = info._depth_clamp;
		rasterState.rasterizerDiscardEnable = VK_FALSE;
		rasterState.lineWidth = 1.f;
		rasterState.depthBiasEnable = VK_FALSE;

		VkPipelineColorBlendStateCreateInfo blendState = {};
		if (info._blendAttachmentStates.size() == 0)
			info.addBlendAttachmentState(false);

		blendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		blendState.logicOpEnable = VK_FALSE;
		blendState.logicOp = VK_LOGIC_OP_COPY;
		blendState.attachmentCount = info._blendAttachmentStates.size();
		blendState.pAttachments = info._blendAttachmentStates.data();

		VkPipelineMultisampleStateCreateInfo multisampleState = {};
		multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampleState.sampleShadingEnable = VK_FALSE;
		multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		VkPipelineDynamicStateCreateInfo dynamicState = {};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = info._dynamicStates.size();
		dynamicState.pDynamicStates = info._dynamicStates.data();

		VkGraphicsPipelineCreateInfo pipelineInfo = {};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.layout = pipelineLayout->v;
		pipelineInfo.stageCount = vkStages.size();
		pipelineInfo.pStages = vkStages.data();
		pipelineInfo.pVertexInputState = info._vertex_input;
		pipelineInfo.pInputAssemblyState = &assemblyState;
		pipelineInfo.pTessellationState = info._patch_control_points ? &tessState : nullptr;
		pipelineInfo.pDepthStencilState = &depthStencilState;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterState;
		pipelineInfo.pColorBlendState = &blendState;
		pipelineInfo.renderPass = _renderPass->v;
		pipelineInfo.subpass = _subpassIndex;
		pipelineInfo.pMultisampleState = &multisampleState;
		pipelineInfo.pDynamicState = info._dynamicStates.size() ? &dynamicState : nullptr;

		auto res = vkCreateGraphicsPipelines(vk_device.v, 0, 1, &pipelineInfo, nullptr, &pipeline);
		assert(res == VK_SUCCESS);

		links = info.links;

		if (need_default_ds)
		{
			descriptorSet = new DescriptorSet(this);
			linkDescriptors(descriptorSet, &globalResource);
		}
	}

	Pipeline::~Pipeline()
	{
		delete descriptorSet;

		vkDestroyPipeline(vk_device.v, pipeline, nullptr);
	}

	void Pipeline::linkDescriptors(DescriptorSet *set, Resource *resource)
	{
		std::vector<VkWriteDescriptorSet> writes;
		for (auto &link : links)
		{
			if (link.binding == -1)
			{
				bool found = false;
				for (auto &s : shaders)
				{
					if (found) break;

					for (auto set = 0; set < s->descriptors.size(); set++)
					{
						for (auto &d : s->descriptors[set])
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
			if (link.type == VK_DESCRIPTOR_TYPE_MAX_ENUM)
			{
				bool found = false;
				for (auto &s : shaders)
				{
					if (found) break;

					for (auto set = 0; set < s->descriptors.size(); set++)
					{
						for (auto &d : s->descriptors[set])
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

	int Pipeline::descriptorPosition(const std::string &name)
	{
		for (auto &s : shaders)
		{
			for (auto set = 0; set < s->descriptors.size(); set++)
			{
				for (auto &d : s->descriptors[set])
				{
					if (d.name == name)
						return d.binding;
				}
			}
		}
		return -1;
	}

	VkPipelineVertexInputStateCreateInfo vertexStateInfo(int bindingCount, VkVertexInputBindingDescription *pBindings, int attributeCount, VkVertexInputAttributeDescription *pAttributes)
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
	}
}
