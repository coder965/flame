#pragma once

#include <vector>

#include "vulkan.h"

namespace tke
{
	struct Framebuffer
	{
		int cx, cy;
		std::vector<VkImageView> views;
		VkFramebuffer v;
		int refCount = 1;

		~Framebuffer();
	};

	struct Image;
	struct RenderPass;
	Framebuffer *getFramebuffer(Image *i, RenderPass *renderPass, int level = 0);

	struct RenderPass;
	Framebuffer *getFramebuffer(int cx, int cy, RenderPass *renderPass, int viewCount, VkImageView *views);

	void releaseFramebuffer(Framebuffer *f);

}
