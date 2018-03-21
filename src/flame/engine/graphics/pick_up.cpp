#include <flame/global.h>
#include <flame/engine/core/core.h>
#include <flame/engine/graphics/buffer.h>
#include <flame/engine/graphics/texture.h>
#include <flame/engine/graphics/command_buffer.h>
#include <flame/engine/graphics/framebuffer.h>
#include <flame/engine/graphics/renderpass.h>
#include <flame/engine/graphics/renderer.h>
#include <flame/engine/graphics/pick_up.h>

namespace flame
{
	Texture *pick_up_image = nullptr;
	Texture *pick_up_depth_image = nullptr;
	static std::shared_ptr<RenderPass> renderpass;
	std::shared_ptr<Framebuffer> pick_up_fb;

	unsigned int pick_up(int x, int y, const std::function<void(CommandBuffer*)> &drawCallback)
	{
		if (x < 0 || y < 0 || x > pick_up_image->get_cx() || y > pick_up_image->get_cy())
			return 0;

		auto cb = begin_once_command_buffer();
		cb->begin_renderpass(renderpass.get(), pick_up_fb.get());
		drawCallback(cb);
		cb->end_renderpass();
		end_once_command_buffer(cb);

		pick_up_image->copy_to_buffer(defalut_staging_buffer, 0, x, y, 1, 1);

		defalut_staging_buffer->map(0, 4);
		auto pixel = (unsigned char*)defalut_staging_buffer->mapped;
		unsigned int index = pixel[0] + (pixel[1] << 8) + (pixel[2] << 16) + (pixel[3] << 24);
		defalut_staging_buffer->unmap();

		return index;
	}

	void init_pick_up() 
	{
		if (!only_2d)
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

			renderpass = get_renderpass(RenderPassInfo()
				.add_attachment(VK_FORMAT_R8G8B8A8_UNORM, true)
				.add_attachment(VK_FORMAT_D16_UNORM, true)
				.add_subpass({ 0 }, 1)
			);

			pick_up_fb = get_framebuffer(resolution.x(), resolution.y(), renderpass.get(), TK_ARRAYSIZE(views), views);
		}
	}
}
