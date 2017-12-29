#include "../../global.h"
#include "../../graphics/renderpass.h"
#include "tool.h"

bool Tool::inited = false;
std::shared_ptr<tke::Framebuffer> Tool::fb;
std::unique_ptr<tke::Image> Tool::depthImage;

Tool::Tool(tke::Image *dst)
{
	if (!inited)
	{
		depthImage = std::make_unique<tke::Image>(tke::res_cx, tke::res_cy,
			VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
			VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
		VkImageView views[] = {
			dst->getView(),
			depthImage->getView()
		};
		fb = tke::getFramebuffer(dst->levels[0].cx, dst->levels[0].cy, 
			tke::renderPass_depthC_image8, TK_ARRAYSIZE(views), views);
	}
}

Tool::~Tool() {}

bool Tool::leftDown(int x, int y) { return false; }
void Tool::mouseMove(int xDisp, int yDisp) {}

Tool *currentTool = nullptr;