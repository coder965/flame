//MIT License
//
//Copyright (c) 2018 wjs
//
//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files (the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions:
//
//The above copyright notice and this permission notice shall be included in all
//copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//SOFTWARE.

#include <tuple>
#include <memory>

#include "device_private.h"
#include "renderpass_private.h"

namespace flame
{
	namespace graphics
	{
#if defined(FLAME_GRAPHICS_VULKAN)
		void Renderpass::add_attachment(Format format, bool clear)
		{
			_priv->attachments.emplace_back(format, clear);
		}

		void Renderpass::add_subpass(const std::initializer_list<int> &color_attachments, int depth_attachment)
		{
			_priv->subpasses.emplace_back(color_attachments, depth_attachment);
		}

		void Renderpass::add_dependency(int src_subpass, int dst_subpass)
		{
			_priv->dependencies.emplace_back(src_subpass, dst_subpass);
		}

		void Renderpass::build()
		{
			std::vector<VkAttachmentDescription> vk_attachments(_priv->attachments.size());
			for (auto i = 0; i < vk_attachments.size(); i++)
			{
				vk_attachments[i].flags = 0;
				vk_attachments[i].format = Z(std::get<0>(_priv->attachments[i]));
				vk_attachments[i].samples = VK_SAMPLE_COUNT_1_BIT;
				vk_attachments[i].loadOp = std::get<1>(_priv->attachments[i]) ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				vk_attachments[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
				vk_attachments[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				vk_attachments[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
				vk_attachments[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				
				auto fmt = std::get<0>(_priv->attachments[i]);
				if (fmt >= Format_Color_Begin && fmt <= Format_Color_End)
				{
					if (fmt >= Format_Swapchain_Begin && fmt <= Format_Swapchain_End)
						vk_attachments[i].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
					else
						vk_attachments[i].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
					_priv->clear_values.push_back({});
				}
				else
				{
					vk_attachments[i].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
					_priv->clear_values.push_back({1, 0.f});
				}
			}

			std::vector<std::unique_ptr<VkAttachmentReference[]>> vk_color_refs(_priv->subpasses.size());
			std::vector<std::unique_ptr<VkAttachmentReference>> vk_depth_refs(_priv->subpasses.size());
			std::vector<VkSubpassDescription> vk_subpasses(_priv->subpasses.size());
			for (auto i = 0; i < vk_subpasses.size(); i++)
			{
				vk_subpasses[i].flags = 0;
				vk_subpasses[i].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
				vk_subpasses[i].inputAttachmentCount = 0;
				vk_subpasses[i].pInputAttachments = nullptr;
				vk_subpasses[i].preserveAttachmentCount = 0;
				vk_subpasses[i].pPreserveAttachments = nullptr;
				vk_subpasses[i].pResolveAttachments = nullptr;
				if (_priv->subpasses[i].first.size() > 0)
				{
					vk_color_refs[i] = std::make_unique<VkAttachmentReference[]>(_priv->subpasses[i].first.size());
					for (auto j = 0; j < _priv->subpasses[i].first.size(); j++)
					{
						vk_color_refs[i][j].attachment = _priv->subpasses[i].first[j];
						vk_color_refs[i][j].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
					}
					vk_subpasses[i].colorAttachmentCount = _priv->subpasses[i].first.size();
					vk_subpasses[i].pColorAttachments = vk_color_refs[i].get();
				}
				else
				{
					vk_subpasses[i].colorAttachmentCount = 0;
					vk_subpasses[i].pColorAttachments = nullptr;
				}
				if (_priv->subpasses[i].second != -1)
				{
					vk_depth_refs[i] = std::make_unique<VkAttachmentReference>();
					vk_depth_refs[i]->attachment = _priv->subpasses[i].second;
					vk_depth_refs[i]->layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
					vk_subpasses[i].pDepthStencilAttachment = vk_depth_refs[i].get();
				}
				else
					vk_subpasses[i].pDepthStencilAttachment = nullptr;
			}

			std::vector<VkSubpassDependency> vk_dependencies(_priv->dependencies.size());
			for (auto i = 0; i < vk_dependencies.size(); i++)
			{
				vk_dependencies[i].srcSubpass = _priv->dependencies[i].first;
				vk_dependencies[i].dstSubpass = _priv->dependencies[i].second;
				vk_dependencies[i].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				vk_dependencies[i].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
				vk_dependencies[i].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
				vk_dependencies[i].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
				vk_dependencies[i].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
			}

			VkRenderPassCreateInfo create_info;
			create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			create_info.flags = 0;
			create_info.pNext = nullptr;
			create_info.attachmentCount = vk_attachments.size();
			create_info.pAttachments = vk_attachments.data();
			create_info.subpassCount = vk_subpasses.size();
			create_info.pSubpasses = vk_subpasses.data();
			create_info.dependencyCount = vk_dependencies.size();
			create_info.pDependencies = vk_dependencies.data();

			release();

			vk_chk_res(vkCreateRenderPass(_priv->d->_priv->device, &create_info, nullptr, &_priv->v));

			valid = true;
		}

		void Renderpass::release()
		{
			if (_priv->v)
			{
				vkDestroyRenderPass(_priv->d->_priv->device, _priv->v, nullptr);
				_priv->v = 0;
				valid = false;
			}
		}

		Renderpass *create_renderpass(Device *d)
		{
			auto r = new Renderpass;
			r->valid = false;

			r->_priv = new RenderpassPrivate;
			r->_priv->d = d;
			r->_priv->v = 0;

			return r;
		}

		void destroy_renderpass(Device *d, Renderpass *r)
		{
			assert(d == r->_priv->d);

			r->release();

			delete r->_priv;
			delete r;
		}
#endif
	}
}

