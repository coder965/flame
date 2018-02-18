#include <flame/global.h>
#include <flame/graphics/texture.h>
#include <flame/graphics/renderpass.h>
#include <flame/graphics/framebuffer.h>
#include <flame/graphics/renderer.h>
#include <flame/graphics/display_layer.h>

namespace tke
{
	bool DisplayLayer::on_message(Object *sender, Message msg)
	{
		switch (msg)
		{
			case MessageResolutionChange:
				create();
				return true;
		}
	}

	DisplayLayer::DisplayLayer(bool _enable_depth) :
		enable_depth(_enable_depth)
	{
		follow_to(&resolution);
		create();
	}

	void DisplayLayer::create()
	{
		if (image && image->get_cx() == resolution.x() && image->get_cy() == resolution.y())
			return;

		image = std::make_shared<Texture>(resolution.x(), resolution.y(), VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
		if (enable_depth)
		{
			depth_image = std::make_unique<Texture>(resolution.x(), resolution.y(), VK_FORMAT_D16_UNORM, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
			VkImageView views[] = {
				image->get_view(),
				depth_image->get_view()
			};
			framebuffer = getFramebuffer(resolution.x(), resolution.y(), renderPass_depthC_image8, TK_ARRAYSIZE(views), views);
		}
		else
			framebuffer = getFramebuffer(image.get(), renderPass_image8);
	}
}
