#ifndef __TKE_PICKUP__
#define __TKE_PICKUP__

#include "render.h"

namespace tke
{
	extern Pipeline plainPickUpPipeline;
	//extern Renderer plainPickUpRenderer;
	unsigned int pickUp(int x, int y, int cx, int cy, void(*drawCallback)(VkCommandBuffer));
	void initPickUp();
}

#endif