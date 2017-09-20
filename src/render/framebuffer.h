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

		~Framebuffer();
	};

	struct Image;
	struct RenderPass;

	std::shared_ptr<Framebuffer> getFramebuffer(Image *i, RenderPass *renderPass, int level = 0);
	std::shared_ptr<Framebuffer> getFramebuffer(int cx, int cy, RenderPass *renderPass, int viewCount, VkImageView *views);
}
