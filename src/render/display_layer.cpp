#include "display_layer.h"
#include "../core.h"
#include "../ui/ui.h"

namespace tke
{
	DisplayLayer::DisplayLayer()
	{
		image = std::make_shared<tke::Image>(resCx, resCy, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
		framebuffer = tke::getFramebuffer(image.get(), renderPass_image8);
		addUiImage(image.get());
	}

	DisplayLayer::~DisplayLayer()
	{
		tke::removeUiImage(image.get());
	}
}
