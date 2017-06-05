#include "pickUp.h"
#include "render.h"
#include "core.h"

namespace tke
{
	static Image *image = nullptr;
	static VkRenderPass renderPass;
	static Framebuffer *framebuffer;

	Pipeline plainPickUpPipeline;

	unsigned int pickUp(int x, int y, int cx, int cy, void(*drawCallback)(VkCommandBuffer))
	{
		if (x + cx > image->m_width || y + cy > image->m_height)
			return 0;

		graphicsQueue.waitIdle();

		auto cmd = commandPool.begineOnce();

		VkClearValue clearValue[2] = {
			{ 0.f, 0.f, 0.f, 0.f },
			{ 1.f, 0 }
		};
		vkCmdBeginRenderPass(cmd, &renderPassBeginInfo(renderPass, framebuffer->v, resCx, resCy, 2, clearValue), VK_SUBPASS_CONTENTS_INLINE);
		
		drawCallback(cmd);

		vkCmdEndRenderPass(cmd);

		commandPool.endOnce(cmd);

		cmd = commandPool.begineOnce();

		VkBufferImageCopy range = {};
		range.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		range.imageSubresource.layerCount = 1;
		range.imageOffset.x = x;
		range.imageOffset.y = y;
		range.imageExtent.width = cx;
		range.imageExtent.height = cy;
		range.imageExtent.depth = 1;

		vkCmdCopyImageToBuffer(cmd, image->m_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, stagingBuffer->m_buffer, 1, &range);

		commandPool.endOnce(cmd);

		auto pixel = (unsigned char*)stagingBuffer->map(0, cx * cy * 4);
		unsigned int index = pixel[0] + (pixel[1] << 8) + (pixel[2] << 16) + (pixel[3] << 24);

		stagingBuffer->unmap();

		return index;
	}

	void initPickUp()
	{
		image = new Image(resCx, resCy, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
		globalResource.setImage(image, "PickUp.Image");

		globalResource.setPipeline(&plainPickUpPipeline, "PickUp.Pipeline");

		auto pDepthImage = globalResource.getImage("Depth.Image");

		VkAttachmentDescription attachments[] = {
			colorAttachmentDesc(image->m_format, VK_ATTACHMENT_LOAD_OP_CLEAR), // pickup image
			depthAttachmentDesc(pDepthImage->m_format, VK_ATTACHMENT_LOAD_OP_CLEAR) // depth
		};

		VkAttachmentReference colorRef = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };

		VkAttachmentReference depthRef = { 1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };

		renderPass = createRenderPass(ARRAYSIZE(attachments), attachments, 1, &subpassDesc(1, &colorRef, &depthRef), 0, nullptr);

		std::vector<VkImageView> views = { image->getView(), pDepthImage->getView() };
		framebuffer = createFramebuffer(resCx, resCy, renderPass, views);

		plainPickUpPipeline.create(enginePath + "pipeline/pickUp/pickUp.xml", &vertexInputState, renderPass, 0);
	}
}
