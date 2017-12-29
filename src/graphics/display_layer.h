#pragma once

#include <memory>

namespace tke
{
	struct Image;
	struct Framebuffer;

	struct DisplayLayer
	{
		std::unique_ptr<Image> image;
		std::unique_ptr<Image> depth_image;
		std::shared_ptr<Framebuffer> framebuffer;

		DisplayLayer(bool enable_depth = false);
		~DisplayLayer();
	};
}
