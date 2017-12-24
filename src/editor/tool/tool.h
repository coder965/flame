#pragma once

#include "../../math/math.h"
#include "../../core.h"
#include "../../entity/camera.h"
#include "../../render/command_buffer.h"
#include "../../render/framebuffer.h"

struct Tool
{
	tke::Framebuffer *fb;

	Tool(tke::Framebuffer *_fb);
	virtual bool leftDown(int x, int y);
	virtual void mouseMove(int xDisp, int yDisp);
	virtual void show(tke::Camera *camera) = 0;
	virtual ~Tool();
};

extern Tool *currentTool;