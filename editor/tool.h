#pragma once

#include "../src/math.h"
#include "../src/render.h"
#include "../src/camera.h"

struct Tool
{
	tke::CommandBuffer *cb;
	tke::Framebuffer *fb;
	VkEvent renderFinished;

	Tool(tke::Framebuffer *_fb);
	virtual void show(tke::Camera *camera, VkEvent waitEvent) = 0;
	virtual ~Tool();
};

extern Tool *currentTool;