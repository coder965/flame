#include "../global.h"
#include "../graphics/buffer.h"
#include "../graphics/texture.h"
#include "../graphics/command_buffer.h"
#include "../graphics/framebuffer.h"
#include "../graphics/renderpass.h"
#include "../graphics/renderer.h"

#include "pick_up.h"

namespace tke
{
	Texture *pick_up_image = nullptr;
	Texture *pick_up_depth_image = nullptr;
	std::shared_ptr<Framebuffer> pick_up_fb;

	unsigned int pick_up(int x, int y, const std::function<void(CommandBuffer*)> &drawCallback)
	{
		if (x < 0 || y < 0 || x > pick_up_image->get_cx() || y > pick_up_image->get_cy())
			return 0;

		auto cb = begin_once_command_buffer();
		cb->beginRenderPass(renderPass_depthC_image8C, pick_up_fb.get());
		drawCallback(cb);
		cb->endRenderPass();
		end_once_command_buffer(cb);

		pick_up_image->copy_to_buffer(defalut_staging_buffer, 0, x, y, 1, 1);

		auto pixel = (unsigned char*)defalut_staging_buffer->map(0, 4);
		unsigned int index = pixel[0] + (pixel[1] << 8) + (pixel[2] << 16) + (pixel[3] << 24);
		defalut_staging_buffer->unmap();

		return index;
	}

	void init_pick_up() 
	{
		pick_up_image = new Texture(resolution.x(), resolution.y(), VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | 
			VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
		pick_up_depth_image = new Texture(resolution.x(), resolution.y(), VK_FORMAT_D16_UNORM,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

		VkImageView views[] = {
			pick_up_image->get_view(),
			pick_up_depth_image->get_view()
		};
		pick_up_fb = getFramebuffer(resolution.x(), resolution.y(), renderPass_depthC_image8C, TK_ARRAYSIZE(views), views);
	}
}
