#pragma once

#include "../src/render.h"

struct Tool
{
	virtual void show(VkCommandBuffer cmd, VkEvent waitEvent, VkEvent signalEvent, VkFramebuffer fb) = 0;
	virtual ~Tool();
};

extern Tool *currentTool;