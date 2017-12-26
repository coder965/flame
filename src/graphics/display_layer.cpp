#include "display_layer.h"
#include "../core.h"
#include "../ui/ui.h"

namespace tke
{
	DisplayLayer::DisplayLayer(bool enable_depth)
	{
		image = std::make_unique<tke::Image>(resCx, resCy, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
		addUiImage(image.get());
		if (enable_depth)
		{
			depth_image = std::make_unique<Image>(resCx, resCy, VK_FORMAT_D16_UNORM, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
			VkImageView views[] = {
				image->getView(),
				depth_image->getView()
			};
			framebuffer = getFramebuffer(resCx, resCy, renderPass_depthC_image8, ARRAYSIZE(views), views);
		}
		else
			framebuffer = tke::getFramebuffer(image.get(), renderPass_image8);
	}

	DisplayLayer::~DisplayLayer()
	{
		tke::removeUiImage(image.get());
	}
}
