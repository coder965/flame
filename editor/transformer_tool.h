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
	virtual void show(const glm::mat4 &matView, VkEvent waitEvent) override;
};
