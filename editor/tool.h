#pragma once

#include "../src/render.h"

struct Tool
{
	virtual void show(VkCommandBuffer) = 0;
	virtual ~Tool();
};

extern Tool *currentTool;