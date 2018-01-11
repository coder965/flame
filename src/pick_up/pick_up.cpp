#include "../type.h"
#include "../global.h"
#include "../graphics/buffer.h"
#include "../graphics/image.h"
#include "../graphics/command_buffer.h"
#include "../graphics/framebuffer.h"
#include "../graphics/renderpass.h"

#include "pick_up.h"

namespace tke
{
	Image *pick_up_image = nullptr;
	Image *pick_up_depth_image = nullptr;
	std::shared_ptr<Framebuffer> pick_up_fb;

	unsigned int pick_up(int x, int y, const std::function<void(CommandBuffer*)> &drawCallback)
	{
		if (x < 0 || y < 0 || x > pick_up_image->levels[0].cx || y > pick_up_image->levels[0].cy)
			return 0;

		auto cb = begineOnceCommandBuffer();
		cb->beginRenderPass(renderPass_depthC_image8C, pick_up_fb.get());
		drawCallback(cb);
		cb->endRenderPass();
		endOnceCommandBuffer(cb);

		cb = begineOnceCommandBuffer();
		VkBufferImageCopy range = {};
		range.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		range.imageSubresource.layerCount = 1;
		range.imageOffset.x = x;
		range.imageOffset.y = y;
		range.imageExtent.width = 1;
		range.imageExtent.height = 1;
		range.imageExtent.depth = 1;
		vkCmdCopyImageToBuffer(cb->v, pick_up_image->v, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, defalut_staging_buffer->v, 1, &range);
		endOnceCommandBuffer(cb);

		auto pixel = (unsigned char*)defalut_staging_buffer->map(0, 4);
		unsigned int index = pixel[0] + (pixel[1] << 8) + (pixel[2] << 16) + (pixel[3] << 24);
		defalut_staging_buffer->unmap();

		return index;
	}

	void init_pick_up() 
	{
		pick_up_image = new Image(res_cx, res_cy, VK_FORMAT_R8G8B8A8_UNORM, 
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | 
			VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
		pick_up_depth_image = new Image(res_cx, res_cy, VK_FORMAT_D16_UNORM, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

		VkImageView views[] = {
			pick_up_image->getView(),
			pick_up_depth_image->getView()
		};
		pick_up_fb = getFramebuffer(res_cx, res_cy, renderPass_depthC_image8C, TK_ARRAYSIZE(views), views);
	}
}
