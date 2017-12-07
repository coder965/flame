#pragma once

#include "../../../src/math/math.h"
#include "../../../src/core.h"
#include "../../../src/entity/camera.h"
#include "../../../src/render/command_buffer.h"
#include "../../../src/render/framebuffer.h"

struct Tool
{
	tke::Framebuffer *fb;

	Tool(tke::Framebuffer *_fb);
	virtual bool leftDown(int x, int y);
	virtual void mouseMove(int xDisp, int yDisp);
	virtual void show(tke::FrameCommandBufferList *cb_list, tke::Camera *camera) = 0;
	virtual ~Tool();
};

extern Tool *currentTool;