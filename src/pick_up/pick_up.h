#pragma once

#include <memory>
#include <functional>

#include "../refl.h"

namespace tke
{
	struct Image;
	struct Framebuffer;
	struct CommandBuffer;

	IMPL(nullptr) Image *pick_up_image;
	IMPL(nullptr) Image *pick_up_depth_image;
	IMPL() std::shared_ptr<Framebuffer> pick_up_fb;

	unsigned int pick_up(int x, int y, const std::function<void(CommandBuffer*)> &drawCallback);

	void init_pick_up();
}
