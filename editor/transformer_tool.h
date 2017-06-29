#pragma once

#include "tool.h"

struct TransformerTool : Tool
{
	virtual void show(VkCommandBuffer cmd, VkEvent waitEvent, VkEvent signalEvent, VkFramebuffer fb) override;
};