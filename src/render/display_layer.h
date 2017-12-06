#pragma once

#include "image.h"
#include "framebuffer.h"

namespace tke
{
	struct DisplayLayer
	{
		std::shared_ptr<Image> image;
		std::shared_ptr<Framebuffer> framebuffer;

		DisplayLayer();
		~DisplayLayer();
	};
}
