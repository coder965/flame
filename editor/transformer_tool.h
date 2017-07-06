#pragma once

#include "..\src\transformer.h"

#include "tool.h"

struct TransformerTool : Tool
{
	enum Mode
	{
		ModeNull,
		ModeMove,
		ModeRotate,
		ModeScale
	};

	tke::Transformer *transformer = nullptr;
	Mode mode = ModeNull;

	TransformerTool(tke::Framebuffer *_fb);
	virtual bool TransformerTool::leftDown(int x, int y) override;
	virtual void show(tke::Camera *camera, VkEvent waitEvent) override;
};
