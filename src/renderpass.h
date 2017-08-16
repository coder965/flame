#pragma once

#include <vector>

#include "vulkan.h"

namespace tke
{
	struct RenderPass
	{
		VkRenderPass v;
		std::vector<VkClearValue> clearValues;

		RenderPass(int attachmentCount, VkAttachmentDescription *pAttachments, int subpassCount, VkSubpassDescription *pSubpasses, int dependencyCount = 0, VkSubpassDependency *pDependencies = nullptr);
		~RenderPass();
	};
}
