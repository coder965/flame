#pragma once

#include "../src/math.h"
#include "../src/render.h"

struct Tool
{
	tke::CommandBuffer *cb;
	tke::Framebuffer *fb;
	VkEvent renderFinished;

	Tool(tke::Framebuffer *_fb);
	virtual void show(const glm::mat4 &matView, VkEvent waitEvent) = 0;
	virtual ~Tool();
};

extern Tool *currentTool;