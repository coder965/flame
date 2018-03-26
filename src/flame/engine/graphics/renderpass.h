#pragma once

#include <vector>

#include <flame/engine/graphics/graphics.h>

namespace flame
{
	struct RenderPassInfo
	{
		std::vector<std::pair<VkFormat, bool>> attachments;
		std::vector<std::pair<std::vector<int>, int>> subpasses;
		std::vector<std::pair<int, int>> dependencies;

		RenderPassInfo &add_attachment(VkFormat format, bool clear);
		RenderPassInfo &add_subpass(const std::initializer_list<int> &color_attachments, int depth_attachment);
		RenderPassInfo &add_dependency(int src_subpass, int dst_subpass);
	};

	bool operator==(const RenderPassInfo &lhs, const RenderPassInfo &rhs);

	struct RenderPass
	{
		RenderPassInfo info;
		std::vector<VkClearValue> clear_values;

		VkRenderPass v;

		// must call in main thread
		RenderPass(const RenderPassInfo &_info);
		// must call in main thread
		~RenderPass();
	};

	std::shared_ptr<RenderPass> get_renderpass(const RenderPassInfo &_info);
}
