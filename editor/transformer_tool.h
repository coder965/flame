#pragma once

#include "tool.h"

struct TransformerTool : Tool
{
	TransformerTool(tke::Framebuffer *_fb);
	virtual void show(VkEvent waitEvent) override;
};
