#include <assert.h>
#include <memory>

#include <flame/global.h>
#include <flame/engine/core/core.h>
#include <flame/engine/graphics/renderpass.h>

namespace flame
{
	RenderPassInfo &RenderPassInfo::add_attachment(VkFormat format, bool clear)
	{
		attachments.emplace_back(format, clear);
		return *this;
	}

	RenderPassInfo &RenderPassInfo::add_subpass(const std::initializer_list<int> &color_attachments, int depth_attachment)
	{
		subpasses.emplace_back(color_attachments, depth_attachment);
		return *this;
	}

	RenderPassInfo &RenderPassInfo::add_dependency(int src_subpass, int dst_subpass)
	{
		dependencies.emplace_back(src_subpass, dst_subpass);
		return *this;
	}

	RenderPass::RenderPass(const RenderPassInfo &_info)
	{
		info = _info;

		std::vector<VkAttachmentDescription> vk_attachments(info.attachments.size());
		for (auto i = 0; i < vk_attachments.size(); i++)
		{
			vk_attachments[i].flags = 0;
			vk_attachments[i].format = info.attachments[i].first == VK_FORMAT_UNDEFINED ? swapchain_format : info.attachments[i].first;
			vk_attachments[i].samples = VK_SAMPLE_COUNT_1_BIT;
			vk_attachments[i].loadOp = std::get<1>(info.attachments[i]) ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			vk_attachments[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			vk_attachments[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			vk_attachments[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			if (info.attachments[i].first == VK_FORMAT_UNDEFINED)
			{
				vk_attachments[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				vk_attachments[i].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			}
			else
			{
				switch (get_format_type(std::get<0>(info.attachments[i])))
				{
					case FormatTypeColor:
					{
						vk_attachments[i].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
						vk_attachments[i].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
						clear_values.push_back({});
						break;
					}
					case FormatTypeDepth: case FormatTypeDepthStencil:
					{
						vk_attachments[i].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
						vk_attachments[i].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
						clear_values.push_back({ 1, 0.f });
						break;
					}
				}
			}
		}

		std::vector<std::unique_ptr<VkAttachmentReference[]>> vk_color_refs(info.subpasses.size());
		std::vector<std::unique_ptr<VkAttachmentReference>> vk_depth_refs(info.subpasses.size());
		std::vector<VkSubpassDescription> vk_subpasses(info.subpasses.size());
		for (auto i = 0; i < vk_subpasses.size(); i++)
		{
			vk_subpasses[i].flags = 0;
			vk_subpasses[i].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			vk_subpasses[i].inputAttachmentCount = 0;
			vk_subpasses[i].pInputAttachments = nullptr;
			vk_subpasses[i].preserveAttachmentCount = 0;
			vk_subpasses[i].pPreserveAttachments = nullptr;
			vk_subpasses[i].pResolveAttachments = nullptr;
			if (info.subpasses[i].first.size() > 0)
			{
				vk_color_refs[i] = std::make_unique<VkAttachmentReference[]>(info.subpasses[i].first.size());
				for (auto j = 0; j < info.subpasses[i].first.size(); j++)
				{
					vk_color_refs[i][j].attachment = info.subpasses[i].first[j];
					vk_color_refs[i][j].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				}
				vk_subpasses[i].colorAttachmentCount = info.subpasses[i].first.size();
				vk_subpasses[i].pColorAttachments = vk_color_refs[i].get();
			}
			else
			{
				vk_subpasses[i].colorAttachmentCount = 0;
				vk_subpasses[i].pColorAttachments = nullptr;
			}
			if (info.subpasses[i].second != -1)
			{
				vk_depth_refs[i] = std::make_unique<VkAttachmentReference>();
				vk_depth_refs[i]->attachment = info.subpasses[i].second;
				vk_depth_refs[i]->layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				vk_subpasses[i].pDepthStencilAttachment = vk_depth_refs[i].get();
			}
			else
				vk_subpasses[i].pDepthStencilAttachment = nullptr;
		}
		
		std::vector<VkSubpassDependency> vk_dependencies(info.dependencies.size());
		for (auto i = 0; i < vk_dependencies.size(); i++)
		{
			vk_dependencies[i].srcSubpass = info.dependencies[i].first;
			vk_dependencies[i].dstSubpass = info.dependencies[i].second;
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

		vk_chk_res(vkCreateRenderPass(vk_device, &create_info, nullptr, &v));
	}

	RenderPass::~RenderPass()
	{
		vkDestroyRenderPass(vk_device, v, nullptr);
	}

	bool operator==(const RenderPassInfo &lhs, const RenderPassInfo &rhs)
	{
		if (lhs.attachments.size() != rhs.attachments.size())
			return false;
		if (lhs.subpasses.size() != rhs.subpasses.size())
			return false;
		if (lhs.dependencies.size() != rhs.dependencies.size())
			return false;
		for (auto i = 0; i < lhs.attachments.size(); i++)
		{
			if (lhs.attachments[i].first != rhs.attachments[i].first)
				return false;
			if (lhs.attachments[i].second != rhs.attachments[i].second)
				return false;
		}
		for (auto i = 0; i < lhs.subpasses.size(); i++)
		{
			if (lhs.subpasses[i].first.size() != rhs.subpasses[i].first.size())
				return false;
			for (auto j = 0; j < lhs.subpasses[i].first.size(); j++)
			{
				if (lhs.subpasses[i].first[j] != rhs.subpasses[i].first[j])
					return false;
			}
			if (lhs.subpasses[i].second != rhs.subpasses[i].second)
				return false;
		}
		for (auto i = 0; i < lhs.dependencies.size(); i++)
		{
			if (lhs.dependencies[i].first != rhs.dependencies[i].first)
				return false;
			if (lhs.dependencies[i].second != rhs.dependencies[i].second)
				return false;
		}
		return true;
	}

	static std::vector<std::weak_ptr<RenderPass>> _renderpasses;

	std::shared_ptr<RenderPass> get_renderpass(const RenderPassInfo &_info)
	{
		for (auto it = _renderpasses.begin(); it != _renderpasses.end(); )
		{
			auto s = it->lock();
			if (s)
			{
				if (s->info == _info)
					return s;
				it++;
			}
			else
				it = _renderpasses.erase(it);
		}
		auto s = std::make_shared<RenderPass>(_info);
		_renderpasses.push_back(s);
		return s;
	}
}
