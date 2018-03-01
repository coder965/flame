#pragma once

#include <flame/entity/camera.h>

struct Tool
{
	Tool();
	virtual void show(flame::CameraComponent *camera) = 0;
	virtual ~Tool() {};
};
