#pragma once

#include <vector>
#include <memory>

#include <flame/engine/graphics/graphics.h>

namespace flame
{
	struct Texture;
	struct RenderPass;

	struct Framebuffer
	{
		int cx, cy;
		std::vector<VkImageView> views;

		VkFramebuffer v;

		// must call in main thread
		~Framebuffer();
	};

	// must call in main thread
	std::shared_ptr<Framebuffer> get_framebuffer(Texture *i, RenderPass *renderpass, int level = 0);
	// must call in main thread
	std::shared_ptr<Framebuffer> get_framebuffer(int cx, int cy, RenderPass *renderpass, int view_count, VkImageView *views);
}
