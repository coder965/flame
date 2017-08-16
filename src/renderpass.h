#pragma once

#include <vector>

#include "vulkan.h"

namespace tke
{
	VkAttachmentDescription colorAttachmentDesc(VkFormat format, VkAttachmentLoadOp loadOp);
	VkAttachmentDescription depthAttachmentDesc(VkFormat format, VkAttachmentLoadOp loadOp);
	VkAttachmentDescription swapchainAttachmentDesc(VkAttachmentLoadOp loadOp);
	VkSubpassDescription subpassDesc(int colorCount, VkAttachmentReference *pColors, VkAttachmentReference *pDepth = nullptr, int inputCount = 0, VkAttachmentReference *pInputs = nullptr);
	VkSubpassDependency subpassDependency(int srcSubpass, int dstSubpass);

	struct RenderPass
	{
		VkRenderPass v;
		std::vector<VkClearValue> clearValues;

		RenderPass(int attachmentCount, VkAttachmentDescription *pAttachments, int subpassCount, VkSubpassDescription *pSubpasses, int dependencyCount = 0, VkSubpassDependency *pDependencies = nullptr);
		~RenderPass();
	};
}
