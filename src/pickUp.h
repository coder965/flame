#pragma once

#include "render.h"

namespace tke
{
	extern Pipeline plainPickUpPipeline;
	//extern Renderer plainPickUpRenderer;
	unsigned int pickUp(int x, int y, int cx, int cy, void(*drawCallback)(VkCommandBuffer));
	void initPickUp();
}
