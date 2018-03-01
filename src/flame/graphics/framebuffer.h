#pragma once

#include <vector>
#include <memory>

#include <flame/graphics/graphics.h>

namespace flame
{
	struct Framebuffer
	{
		int cx, cy;
		std::vector<VkImageView> views;
		VkFramebuffer v;

		// must call in main thread
		~Framebuffer();
	};

	struct Texture;
	struct RenderPass;

	// must call in main thread
	std::shared_ptr<Framebuffer> getFramebuffer(Texture *i, RenderPass *renderPass, int level = 0);
	// must call in main thread
	std::shared_ptr<Framebuffer> getFramebuffer(int cx, int cy, RenderPass *renderPass, int viewCount, VkImageView *views);
}
