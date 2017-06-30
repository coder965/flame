#pragma once

#include "../src/render.h"

struct Tool
{
	tke::CommandBuffer *cb;
	tke::Framebuffer *fb;
	VkEvent event;

	Tool(tke::Framebuffer *_fb);
	virtual void show(VkEvent waitEvent) = 0;
	virtual ~Tool();
};

extern Tool *currentTool;