#include <assert.h>

#include "renderpass.h"

namespace tke
{
	RenderPass::RenderPass(int attachmentCount, VkAttachmentDescription *pAttachments, int subpassCount, VkSubpassDescription *pSubpasses, int dependencyCount, VkSubpassDependency *pDependencies)
	{
		for (int i = 0; i < attachmentCount; i++)
		{
			if (pAttachments[i].finalLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL ||
				pAttachments[i].finalLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
				clearValues.push_back({});
			else
				clearValues.push_back({ 1, 0.f });
		}

		VkRenderPassCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		info.attachmentCount = attachmentCount;
		info.pAttachments = pAttachments;
		info.subpassCount = subpassCount;
		info.pSubpasses = pSubpasses;
		info.dependencyCount = dependencyCount;
		info.pDependencies = pDependencies;

		device.mtx.lock();
		auto res = vkCreateRenderPass(device.v, &info, nullptr, &v);
		assert(res == VK_SUCCESS);
		device.mtx.unlock();
	}

	RenderPass::~RenderPass()
	{
		device.mtx.lock();
		vkDestroyRenderPass(device.v, v, nullptr);
		device.mtx.unlock();
	}

}
