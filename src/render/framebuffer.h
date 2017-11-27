#pragma once

#include <vector>
#include <memory>

#include "vulkan.h"

namespace tke
{
	struct Framebuffer
	{
		int cx, cy;
		std::vector<VkImageView> views;
		VkFramebuffer v;

		// must call in main thread
		~Framebuffer();
	};

	struct Image;
	struct RenderPass;

	// must call in main thread
	std::shared_ptr<Framebuffer> getFramebuffer(Image *i, RenderPass *renderPass, int level = 0);
	// must call in main thread
	std::shared_ptr<Framebuffer> getFramebuffer(int cx, int cy, RenderPass *renderPass, int viewCount, VkImageView *views);
}
