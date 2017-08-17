#pragma once

#include "../src/math.h"
#include "../src/camera.h"
#include "../src/command_buffer.h"
#include "../src/framebuffer.h"

struct Tool
{
	tke::CommandBuffer *cb;
	tke::Framebuffer *fb;

	Tool(tke::Framebuffer *_fb);
	virtual bool leftDown(int x, int y);
	virtual void mouseMove(int xDisp, int yDisp);
	virtual void show(tke::Camera *camera, VkEvent waitEvent, VkEvent signalEvent) = 0;
	virtual ~Tool();
};

extern Tool *currentTool;