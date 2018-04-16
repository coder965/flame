#include "pipeline_private.h"
#include "device_private.h"
#include "shader_private.h"
#include "renderpass_private.h"

namespace flame
{
	namespace graphics
	{
		void Pipeline::set_size(int cx, int cy)
		{
			_priv->cx = cx;
			_priv->cy = cy;
		}

		void Pipeline::set_vertex_attributes(const std::initializer_list<std::initializer_list<VertexAttributeType>> &attributes)
		{
			_priv->vertex_attributes.resize(attributes.size());
			auto index = 0;
			for (auto &v : attributes)
			{
				_priv->vertex_attributes[index] = v;
				index++;
			}
		}

		void Pipeline::set_patch_control_points(int v)
		{
			_priv->patch_control_points = v;
		}

		void Pipeline::set_depth_test(bool v)
		{
			_priv->depth_test = v;
		}

		void Pipeline::set_depth_write(bool v)
		{
			_priv->depth_write = v;
		}

		void Pipeline::set_depth_clamp(bool v)
		{
			_priv->depth_clamp = v;
		}

		void Pipeline::set_primitive_topology(PrimitiveTopology v)
		{
			_priv->primitive_topology = v;
		}

		void Pipeline::set_polygon_mode(PolygonMode v)
		{
			_priv->polygon_mode = v;
		}

		void Pipeline::set_cull_mode(CullMode v)
		{
			_priv->cull_mode = v;
		}

		void Pipeline::set_output_attachment_count(int count)
		{
			_priv->attachment_blend_states.resize(count);
		}

		void Pipeline::set_blend_state(int index, bool enable,
			BlendFactor src_color, BlendFactor dst_color,
			BlendFactor src_alpha, BlendFactor dst_alpha)
		{
			assert(index < _priv->attachment_blend_states.size());

			_priv->attachment_blend_states[index].enable = enable;
			_priv->attachment_blend_states[index].src_color = src_color;
			_priv->attachment_blend_states[index].dst_color = dst_color;
			_priv->attachment_blend_states[index].src_alpha = src_alpha;
			_priv->attachment_blend_states[index].dst_alpha = dst_alpha;
		}

		void Pipeline::set_dynamic_state(const  std::initializer_list<DynamicState> &states)
		{
			_priv->dynamic_states = states;
		}

		void Pipeline::add_shader(const std::string &filename,
			const std::initializer_list<std::string> &defines)
		{
			auto s = create_shader(_priv->d, filename, defines);
			assert(s);
			_priv->shaders.emplace_back(s);
		}

		void Pipeline::build()
		{
			std::vector<VkVertexInputBindingDescription> vk_vertex_input_state_bindings;
			std::vector<VkVertexInputAttributeDescription> vk_vertex_input_state_attributes;
			std::vector<VkPipelineShaderStageCreateInfo> vk_stage_infos;
			std::vector<VkPipelineColorBlendAttachmentState> vk_blend_attachment_states;
			std::vector<VkDynamicState> vk_dynamic_states;

			for (auto s : _priv->shaders)
			{
				s->build();

				VkPipelineShaderStageCreateInfo info;
				info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
				info.flags = 0;
				info.pNext = nullptr;
				info.pSpecializationInfo = nullptr;
				info.pName = "main";
				info.stage = Z(s->type);
				info.module = s->_priv->v;
				vk_stage_infos.push_back(info);
			}

			auto cx = _priv->cx, cy = _priv->cy;

			if (_priv->cx == -1)
				cx = _priv->d->resolution_x;
			if (_priv->cy == -1)
				cy = _priv->d->resolution_y;

			if (_priv->cx == 0 && _priv->cy == 0)
			{
				if (std::find(_priv->dynamic_states.begin(), _priv->dynamic_states.end(),
					DynamicStateViewport) == _priv->dynamic_states.end())
					_priv->dynamic_states.push_back(DynamicStateViewport);
				if (std::find(_priv->dynamic_states.begin(), _priv->dynamic_states.end(),
					DynamicStateScissor) == _priv->dynamic_states.end())
					_priv->dynamic_states.push_back(DynamicStateScissor);
			}

			VkPipelineInputAssemblyStateCreateInfo assembly_state;
			assembly_state.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
			assembly_state.flags = 0;
			assembly_state.pNext = nullptr;
			assembly_state.topology = Z(_priv->primitive_topology);
			assembly_state.primitiveRestartEnable = VK_FALSE;

			VkPipelineTessellationStateCreateInfo tess_state = {};
			tess_state.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
			tess_state.flags = 0;
			tess_state.pNext = nullptr;
			tess_state.patchControlPoints = _priv->patch_control_points;

			VkPipelineDepthStencilStateCreateInfo depth_stencil_state;
			depth_stencil_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
			depth_stencil_state.flags = 0;
			depth_stencil_state.pNext = nullptr;
			depth_stencil_state.depthTestEnable = _priv->depth_test;
			depth_stencil_state.depthWriteEnable = _priv->depth_write;
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
			raster_state.polygonMode = Z(_priv->polygon_mode);
			raster_state.cullMode = Z(_priv->cull_mode);
			raster_state.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
			raster_state.depthClampEnable = _priv->depth_clamp;
			raster_state.rasterizerDiscardEnable = VK_FALSE;
			raster_state.lineWidth = 1.f;
			raster_state.depthBiasEnable = VK_FALSE;
			raster_state.depthBiasClamp = 0.f;
			raster_state.depthBiasConstantFactor = 0.f;
			raster_state.depthBiasSlopeFactor = 0.f;

			for (auto &b : _priv->attachment_blend_states)
			{
				vk_blend_attachment_states.push_back({b.enable,
					Z(b.src_color), Z(b.dst_color), VK_BLEND_OP_ADD,
					Z(b.src_alpha), Z(b.dst_alpha), VK_BLEND_OP_ADD,
					VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT});
			}

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
			blend_state.attachmentCount = vk_blend_attachment_states.size();
			blend_state.pAttachments = vk_blend_attachment_states.data();

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

			for (auto &s : _priv->dynamic_states)
				vk_dynamic_states.push_back(Z(s));

			VkPipelineDynamicStateCreateInfo dynamic_state = {};
			dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
			dynamic_state.flags = 0;
			dynamic_state.pNext = nullptr;
			dynamic_state.dynamicStateCount = vk_dynamic_states.size();
			dynamic_state.pDynamicStates = vk_dynamic_states.data();

			{
				auto location = 0;
				auto binding = 0;
				for (auto &v : _priv->vertex_attributes)
				{
					auto size = 0;
					for (auto &a : v)
					{
						VkVertexInputAttributeDescription d;
						d.location = location;
						d.binding = binding;
						d.offset = size;
						switch (a)
						{
							case VertexAttributeFloat:
								d.format = VK_FORMAT_R32_SFLOAT;
								size += 4;
								break;
							case VertexAttributeFloat2:
								d.format = VK_FORMAT_R32G32_SFLOAT;
								size += 8;
								break;
							case VertexAttributeFloat3:
								d.format = VK_FORMAT_R32G32B32_SFLOAT;
								size += 12;
								break;
							case VertexAttributeFloat4:
								d.format = VK_FORMAT_R32G32B32A32_SFLOAT;
								size += 16;
								break;
							case VertexAttributeByte4:
								d.format = VK_FORMAT_R8G8B8A8_UNORM;
								size += 4;
								break;
						}
						vk_vertex_input_state_attributes.push_back(d);
						location++;
					}
					VkVertexInputBindingDescription d;
					d.binding = binding;
					d.stride = size;
					d.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
					vk_vertex_input_state_bindings.push_back(d);
					binding++;
				}
			}

			VkPipelineVertexInputStateCreateInfo vertex_input_state;
			vertex_input_state.vertexBindingDescriptionCount = vk_vertex_input_state_bindings.size();
			vertex_input_state.pVertexBindingDescriptions = vk_vertex_input_state_bindings.data();
			vertex_input_state.vertexAttributeDescriptionCount = vk_vertex_input_state_attributes.size();
			vertex_input_state.pVertexAttributeDescriptions = vk_vertex_input_state_attributes.data();

			release();

			VkDescriptorSetLayoutCreateInfo descriptor_set_layout_info;
			descriptor_set_layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			descriptor_set_layout_info.flags = 0;
			descriptor_set_layout_info.pNext = nullptr;
			descriptor_set_layout_info.bindingCount = 0;
			descriptor_set_layout_info.pBindings = nullptr;

			vk_chk_res(vkCreateDescriptorSetLayout(_priv->d->_priv->device, 
				&descriptor_set_layout_info, nullptr, &_priv->descriptor_set_layout));

			VkPipelineLayoutCreateInfo pipeline_layout_info;
			pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			pipeline_layout_info.flags = 0;
			pipeline_layout_info.pNext = nullptr;
			pipeline_layout_info.setLayoutCount = 1;
			pipeline_layout_info.pSetLayouts = &_priv->descriptor_set_layout;
			pipeline_layout_info.pushConstantRangeCount = 0;
			pipeline_layout_info.pPushConstantRanges = nullptr;

			vk_chk_res(vkCreatePipelineLayout(_priv->d->_priv->device, 
				&pipeline_layout_info, nullptr, &_priv->pipeline_layout));

			//for (auto set = 0; set < descriptor_sets.size(); set++)
			//	descriptor_set_layouts[set] = get_descriptor_set_layout(descriptor_sets[set]);

			//pipeline_layout = get_pipeline_layout(descriptor_set_layouts, push_constant_ranges);

			VkGraphicsPipelineCreateInfo pipeline_info;
			pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
			pipeline_info.flags = 0;
			pipeline_info.pNext = nullptr;
			pipeline_info.basePipelineHandle = 0;
			pipeline_info.basePipelineIndex = 0;
			pipeline_info.layout = _priv->pipeline_layout;
			pipeline_info.stageCount = vk_stage_infos.size();
			pipeline_info.pStages = vk_stage_infos.data();
			pipeline_info.pVertexInputState = &vertex_input_state;
			pipeline_info.pInputAssemblyState = &assembly_state;
			pipeline_info.pTessellationState = _priv->patch_control_points > 0 ? &tess_state : nullptr;
			pipeline_info.pDepthStencilState = &depth_stencil_state;
			pipeline_info.pViewportState = &viewport_state;
			pipeline_info.pRasterizationState = &raster_state;
			pipeline_info.pColorBlendState = &blend_state;
			pipeline_info.renderPass = _priv->renderpass->_priv->v;
			pipeline_info.subpass = _priv->subpass_index;
			pipeline_info.pMultisampleState = &multisample_state;
			pipeline_info.pDynamicState = vk_dynamic_states.size() ? &dynamic_state : nullptr;

			vk_chk_res(vkCreateGraphicsPipelines(_priv->d->_priv->device, 0, 1, &pipeline_info, nullptr, &_priv->v));
		}

		void Pipeline::release()
		{
			//_priv->renderpass = nullptr;
			//_priv->subpass_index = -1;
			//_priv->cx = 0;
			//_priv->cy = 0;
			//_priv->patch_control_points = 0;
			//_priv->depth_test = false;
			//_priv->depth_write = false;
			//_priv->depth_clamp = false;
			//_priv->primitive_topology = PrimitiveTopologyTriangleList;
			//_priv->polygon_mode = PolygonModeFill;
			//_priv->cull_mode = CullModeBack;
			//_priv->attachment_blend_states.resize(1);
			//_priv->attachment_blend_states[0].enable = false;
			//_priv->attachment_blend_states[0].src_color = BlendFactorOne;
			//_priv->attachment_blend_states[0].dst_color = BlendFactorZero;
			//_priv->attachment_blend_states[0].src_alpha = BlendFactorOne;
			//_priv->attachment_blend_states[0].dst_alpha = BlendFactorZero;

			if (_priv->descriptor_set_layout)
			{
				vkDestroyDescriptorSetLayout(_priv->d->_priv->device, _priv->descriptor_set_layout, nullptr);
				_priv->descriptor_set_layout = 0;
			}
			if (_priv->pipeline_layout)
			{
				vkDestroyPipelineLayout(_priv->d->_priv->device, _priv->pipeline_layout, nullptr);
				_priv->pipeline_layout = 0;
			}
			if (_priv->v)
			{
				vkDestroyPipeline(_priv->d->_priv->device, _priv->v, nullptr);
				_priv->v = 0;
			}
		}

		Pipeline *create_pipeline(Device *d, Renderpass *renderpass, int subpass_index)
		{
			auto p = new Pipeline;

			p->_priv = new PipelinePrivate;
			p->_priv->d = d;
			p->_priv->renderpass = renderpass;
			p->_priv->subpass_index = subpass_index;
			p->_priv->cx = 0;
			p->_priv->cy = 0;
			p->_priv->patch_control_points = 0;
			p->_priv->depth_test = false;
			p->_priv->depth_write = false;
			p->_priv->depth_clamp = false;
			p->_priv->primitive_topology = PrimitiveTopologyTriangleList;
			p->_priv->polygon_mode = PolygonModeFill;
			p->_priv->cull_mode = CullModeBack;
			p->_priv->attachment_blend_states.resize(1);
			p->_priv->attachment_blend_states[0].enable = false;
			p->_priv->attachment_blend_states[0].src_color = BlendFactorOne;
			p->_priv->attachment_blend_states[0].dst_color = BlendFactorZero;
			p->_priv->attachment_blend_states[0].src_alpha = BlendFactorOne;
			p->_priv->attachment_blend_states[0].dst_alpha = BlendFactorZero;

			p->_priv->descriptor_set_layout = 0;
			p->_priv->pipeline_layout = 0;
			p->_priv->v = 0;

			return p;
		}

		void destroy_pipeline(Device *d, Pipeline *p)
		{
			assert(d == p->_priv->d);

			p->release();

			delete p->_priv;
			delete p;
		}
	}
}
