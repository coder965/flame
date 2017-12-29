#pragma once

#include <memory>

#include "../../math/math.h"
#include "../../entity/camera.h"
#include "../../graphics/image.h"
#include "../../graphics/framebuffer.h"
#include "../../graphics/command_buffer.h"

struct Tool
{
	static bool inited;
	static std::shared_ptr<tke::Framebuffer> fb;
	static std::unique_ptr<tke::Image> depthImage;

	Tool(tke::Image *dst);
	virtual bool leftDown(int x, int y);
	virtual void mouseMove(int xDisp, int yDisp);
	virtual void show(tke::Camera *camera) = 0;
	virtual ~Tool();
};

extern Tool *currentTool;