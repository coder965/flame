#pragma once

#include "../src/math.h"
#include "../src/render.h"

struct Tool
{
	tke::CommandBuffer *cb;
	tke::Framebuffer *fb;
	VkEvent renderFinished;

	Tool(tke::Framebuffer *_fb);
	virtual void show(glm::mat4 &matProj, VkEvent waitEvent) = 0;
	virtual ~Tool();
};

extern Tool *currentTool;