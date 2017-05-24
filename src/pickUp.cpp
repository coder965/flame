#include "pickUp.h"
#include "render.h"
#include "core.h"

namespace tke
{
	Image image;
	VkRenderPass renderPass;
	VkFramebuffer framebuffer;
	Pipeline plainPickUpPipeline;

	unsigned int pickUp(int x, int y, int cx, int cy, void(*drawCallback)(VkCommandBuffer))
	{
		if (x + cx > image.m_width || y + cy > image.m_height)
			return 0;

		vk::queueWaitIdle();

		auto cmd = vk::commandPool.begineOnce();

		VkClearValue clearValue[2] = {
			{ 0.f, 0.f, 0.f, 0.f },
			{ 1.f, 0 }
		};
		vkCmdBeginRenderPass(cmd, &vk::renderPassBeginInfo(renderPass, framebuffer, resCx, resCy, 2, clearValue), VK_SUBPASS_CONTENTS_INLINE);
		
		drawCallback(cmd);

		vkCmdEndRenderPass(cmd);

		vk::commandPool.endOnce(cmd);

		cmd = vk::commandPool.begineOnce();

		VkBufferImageCopy range = {};
		range.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		range.imageSubresource.layerCount = 1;
		range.imageOffset.x = x;
		range.imageOffset.y = y;
		range.imageExtent.width = cx;
		range.imageExtent.height = cy;
		range.imageExtent.depth = 1;

		vkCmdCopyImageToBuffer(cmd, image.m_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, stagingBuffer.m_buffer, 1, &range);

		vk::commandPool.endOnce(cmd);

		auto pixel = (unsigned char*)vk::mapMemory(stagingBuffer.m_memory, 0, cx * cy * 4);
		unsigned int index = pixel[0] + (pixel[1] << 8) + (pixel[2] << 16) + (pixel[3] << 24);

		vk::unmapMemory(stagingBuffer.m_memory);

		return index;
	}

	void initPickUp()
	{
		image.create(resCx, resCy, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
		globalResource.setImage(&image, "PickUp.Image");

		globalResource.setPipeline(&plainPickUpPipeline, "PickUp.Pipeline");

		auto pDepthImage = globalResource.getImage("Depth.Image");

		VkAttachmentDescription attachments[] = {
			vk::colorAttachment(image.m_format, VK_ATTACHMENT_LOAD_OP_CLEAR), // pickup image
			vk::depthAttachment(pDepthImage->m_format, VK_ATTACHMENT_LOAD_OP_CLEAR) // depth
		};

		VkAttachmentReference colorRef = {
			0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
		};

		VkAttachmentReference depthRef = {
			1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
		};

		VkSubpassDescription subpass = {
			vk::subpass(1, &colorRef, &depthRef)
		};

		renderPass = vk::createRenderPass(ARRAYSIZE(attachments), attachments, 1, &subpass, 0, nullptr);

		VkImageView views[] = { image.getView(), pDepthImage->getView() };
		framebuffer = vk::createFramebuffer(resCx, resCy, renderPass, ARRAYSIZE(views), views);

		plainPickUpPipeline.create(enginePath + "pipeline/pickUp/pickUp.xml", &vertexInputState, renderPass, 0);
	}
}
