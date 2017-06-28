#pragma once

#include "tool.h"

struct TransformerTool : Tool
{
	virtual void show(VkCommandBuffer) override;
};