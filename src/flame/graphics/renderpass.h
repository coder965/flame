#pragma once

#include <vector>

#include <flame/graphics/graphics.h>

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

		// must call in main thread
		RenderPass(int attachmentCount, VkAttachmentDescription *pAttachments, int subpassCount, VkSubpassDescription *pSubpasses, int dependencyCount = 0, VkSubpassDependency *pDependencies = nullptr);
		// must call in main thread
		~RenderPass();
	};

	// C means clear

	extern RenderPass *renderPass_image8;
	extern RenderPass *renderPass_image8C;
	extern RenderPass *renderPass_image16;
	extern RenderPass *renderPass_image16C;
	extern RenderPass *renderPass_depthC;
	extern RenderPass *renderPass_depthC_image8;
	extern RenderPass *renderPass_depthC_image8C;
	extern RenderPass *renderPass_depthC_image32fC;
	extern RenderPass *renderPass_window;
	extern RenderPass *renderPass_windowC;

	void init_renderpass();
}