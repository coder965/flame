#pragma once

#include "../src/render.h"

struct Tool
{
	VkCommandBuffer cmd;

	Tool();
	virtual void show() = 0;
	virtual ~Tool();
};

extern Tool *currentTool;