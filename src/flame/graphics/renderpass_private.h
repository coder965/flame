#pragma once

#include "renderpass.h"
#include "graphics_private.h"

namespace flame
{
	namespace graphics
	{
		struct RenderpassPrivate
		{
			Device *d;
			std::vector<std::pair<Format, bool>> attachments;
			std::vector<std::pair<std::vector<int>, int>> subpasses;
			std::vector<std::pair<int, int>> dependencies;
			std::vector<VkClearValue> clear_values;
			VkRenderPass v;
		};
	}
}
