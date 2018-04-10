#pragma once

#include <vector>

#include <flame/engine/graphics/graphics.h>

namespace flame
{
	bool operator==(const RenderPassInfo &lhs, const RenderPassInfo &rhs);

	struct RenderPass
	{

		// must call in main thread
		RenderPass(const RenderPassInfo &_info);
		// must call in main thread
		~RenderPass();
	};

	std::shared_ptr<RenderPass> get_renderpass(const RenderPassInfo &_info);
}
