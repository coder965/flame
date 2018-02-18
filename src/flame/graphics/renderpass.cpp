#include <assert.h>

#include <flame/global.h>
#include <flame/graphics/renderpass.h>

namespace tke
{
	VkAttachmentDescription colorAttachmentDesc(VkFormat format, VkAttachmentLoadOp loadOp)
	{
		VkAttachmentDescription desc = {};
		desc.format = format;
		desc.samples = VK_SAMPLE_COUNT_1_BIT;
		desc.loadOp = loadOp;
		desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		desc.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		desc.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		return desc;
	}

	VkAttachmentDescription depthAttachmentDesc(VkFormat format, VkAttachmentLoadOp loadOp)
	{
		VkAttachmentDescription desc = {};
		desc.format = format;
		desc.samples = VK_SAMPLE_COUNT_1_BIT;
		desc.loadOp = loadOp;
		desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		desc.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		desc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		return desc;
	}

	VkAttachmentDescription swapchainAttachmentDesc(VkAttachmentLoadOp loadOp)
	{
		VkAttachmentDescription desc = {};
		desc.format = swapchain_format;
		desc.samples = VK_SAMPLE_COUNT_1_BIT;
		desc.loadOp = loadOp;
		desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		desc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		return desc;
	}

	VkSubpassDescription subpassDesc(int colorCount, VkAttachmentReference *pColors, VkAttachmentReference *pDepth, int inputCount, VkAttachmentReference *pInputs)
	{
		VkSubpassDescription desc = {};
		desc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		desc.inputAttachmentCount = inputCount;
		desc.pInputAttachments = pInputs;
		desc.colorAttachmentCount = colorCount;
		desc.pColorAttachments = pColors;
		desc.pDepthStencilAttachment = pDepth;
		return desc;
	}

	VkSubpassDependency subpassDependency(int srcSubpass, int dstSubpass)
	{
		VkSubpassDependency dependency = {};
		dependency.srcSubpass = srcSubpass;
		dependency.dstSubpass = dstSubpass;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependency.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
		return dependency;
	}

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

		auto res = vkCreateRenderPass(vk_device.v, &info, nullptr, &v);
		assert(res == VK_SUCCESS);
	}

	RenderPass::~RenderPass()
	{
		vkDestroyRenderPass(vk_device.v, v, nullptr);
	}

	RenderPass *renderPass_image8 = nullptr;
	RenderPass *renderPass_image8C = nullptr;
	RenderPass *renderPass_image16 = nullptr;
	RenderPass *renderPass_image16C = nullptr;
	RenderPass *renderPass_depthC = nullptr;
	RenderPass *renderPass_depthC_image8 = nullptr;
	RenderPass *renderPass_depthC_image8C = nullptr;
	RenderPass *renderPass_depthC_image32fC = nullptr;
	RenderPass *renderPass_window = nullptr;
	RenderPass *renderPass_windowC = nullptr;

	void init_renderpass()
	{
		auto att0 = swapchainAttachmentDesc(VK_ATTACHMENT_LOAD_OP_DONT_CARE);
		auto att1 = swapchainAttachmentDesc(VK_ATTACHMENT_LOAD_OP_CLEAR);
		VkAttachmentReference col_ref = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
		VkSubpassDescription subpass0 = subpassDesc(1, &col_ref);
		renderPass_window = new RenderPass(1, &att0, 1, &subpass0);
		renderPass_windowC = new RenderPass(1, &att1, 1, &subpass0);

		if (!only_2d)
		{
			auto att2 = colorAttachmentDesc(VK_FORMAT_R8G8B8A8_UNORM, VK_ATTACHMENT_LOAD_OP_DONT_CARE);
			auto att3 = colorAttachmentDesc(VK_FORMAT_R8G8B8A8_UNORM, VK_ATTACHMENT_LOAD_OP_CLEAR);
			auto att4 = colorAttachmentDesc(VK_FORMAT_R16G16B16A16_SFLOAT, VK_ATTACHMENT_LOAD_OP_DONT_CARE);
			auto att5 = colorAttachmentDesc(VK_FORMAT_R16G16B16A16_SFLOAT, VK_ATTACHMENT_LOAD_OP_CLEAR);
			auto att6 = depthAttachmentDesc(VK_FORMAT_D16_UNORM, VK_ATTACHMENT_LOAD_OP_CLEAR);
			auto att7 = colorAttachmentDesc(VK_FORMAT_R32_SFLOAT, VK_ATTACHMENT_LOAD_OP_CLEAR);
			VkAttachmentReference col_ref = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
			VkAttachmentReference dep_ref0 = { 0, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };
			VkAttachmentReference dep_ref1 = { 1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };
			VkSubpassDescription subpass0 = subpassDesc(1, &col_ref);
			VkSubpassDescription subpass1 = subpassDesc(0, nullptr, &dep_ref0);
			VkSubpassDescription subpass2 = subpassDesc(1, &col_ref, &dep_ref1);
			VkAttachmentDescription atts0[] = {
				att2,
				att6
			};
			VkAttachmentDescription atts1[] = {
				att3,
				att6
			};
			VkAttachmentDescription atts2[] = {
				att7,
				att6
			};
			renderPass_image8 = new RenderPass(1, &att2, 1, &subpass0);
			renderPass_image8C = new RenderPass(1, &att3, 1, &subpass0);
			renderPass_image16 = new RenderPass(1, &att4, 1, &subpass0);
			renderPass_image16C = new RenderPass(1, &att5, 1, &subpass0);
			renderPass_depthC = new RenderPass(1, &att6, 1, &subpass1);
			renderPass_depthC_image8 = new RenderPass(ARRAYSIZE(atts0), atts0, 1, &subpass2);
			renderPass_depthC_image8C = new RenderPass(ARRAYSIZE(atts1), atts1, 1, &subpass2);
			renderPass_depthC_image32fC = new RenderPass(ARRAYSIZE(atts2), atts2, 1, &subpass2);
		}
	}
}
