#include "pickUp.h"
#include "../core/render.h"
#include "../core/core.h"

namespace tke
{
	Image pickUpImage;
	VkRenderPass pickUpRenderPass;
	VkFramebuffer pickUpFramebuffer;
	Pipeline plainPickUpPipeline;

	unsigned int pickUp(int x, int y, int cx, int cy, void(*drawCallback)(VkCommandBuffer, void *), void *userData)
	{
		if (x + cx > pickUpImage.m_width || y + cy > pickUpImage.m_height)
			return 0;

		vk::queueWaitIdle();

		auto cmd = vk::begineOnceCommandBuffer();

		VkClearValue clearValue[2];
		clearValue[0].color = { 0.f, 0.f, 0.f, 0.f };
		clearValue[1].depthStencil = { 1.f, 0 };
		VkRenderPassBeginInfo renderPassBeginInfo = {};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.renderPass = pickUpRenderPass;
		renderPassBeginInfo.framebuffer = pickUpFramebuffer;
		renderPassBeginInfo.renderArea.extent.width = resCx;
		renderPassBeginInfo.renderArea.extent.height = resCy;
		renderPassBeginInfo.clearValueCount = 2;
		renderPassBeginInfo.pClearValues = clearValue;
		vkCmdBeginRenderPass(cmd, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		drawCallback(cmd, userData);

		vkCmdEndRenderPass(cmd);

		vk::endOnceCommandBuffer(cmd);

		cmd = vk::begineOnceCommandBuffer();

		VkBufferImageCopy range = {};
		range.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		range.imageSubresource.layerCount = 1;
		range.imageOffset.x = x;
		range.imageOffset.y = y;
		range.imageExtent.width = cx;
		range.imageExtent.height = cy;
		range.imageExtent.depth = 1;

		vkCmdCopyImageToBuffer(cmd, pickUpImage.m_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, stagingBuffer.m_buffer, 1, &range);

		vk::endOnceCommandBuffer(cmd);

		auto pixel = (unsigned char*)vk::mapMemory(stagingBuffer.m_memory, 0, cx * cy * 4);
		unsigned int index = pixel[0] + (pixel[1] << 8) + (pixel[2] << 16) + (pixel[3] << 24);

		vk::unmapMemory(stagingBuffer.m_memory);

		return index;
	}

	void initPickUp()
	{
		pickUpImage.create(resCx, resCy, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
		globalResource.setImage(&pickUpImage, "PickUp.Image");

		globalResource.setPipeline(&plainPickUpPipeline, "PickUp.Pipeline");

		auto pDepthImage = globalResource.getImage("Depth.Image");

		VkAttachmentDescription attachments[] = {
			vk::colorAttachment(pickUpImage.m_format, VK_ATTACHMENT_LOAD_OP_CLEAR), // pickup image
			vk::depthAttachment(pDepthImage->m_format, VK_ATTACHMENT_LOAD_OP_CLEAR) // depth
		};

		VkAttachmentReference colorReferences[] =
		{
			{ 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }
		};

		VkAttachmentReference depthReferences[] =
		{
			{ 1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL }
		};

		VkSubpassDescription subpass[] = {
			vk::subpass(1, colorReferences, depthReferences)
		};

		pickUpRenderPass = vk::createRenderPass(ARRAYSIZE(attachments), attachments, ARRAYSIZE(subpass), subpass, 0, nullptr);

		VkImageView views[] = { pickUpImage.getView(), pDepthImage->getView() };
		pickUpFramebuffer = vk::createFramebuffer(resCx, resCy, pickUpRenderPass, ARRAYSIZE(views), views);

		plainPickUpPipeline.create("../pipeline/pickUp/pickUp.xml", &vertexInputState, resCx, resCy, pickUpRenderPass, 0);
	}
}
