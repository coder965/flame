#pragma once

#include "../../entity/camera.h"

struct Tool
{
	Tool();
	virtual bool leftDown(int x, int y) { return false; }
	virtual void mouseMove(int xDisp, int yDisp) {}
	virtual void show(tke::CameraComponent *camera) = 0;
	virtual ~Tool();
};

extern Tool *currentTool;