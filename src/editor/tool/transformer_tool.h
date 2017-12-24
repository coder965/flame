#pragma once

#include "../../render/renderer.h"
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
	int selectedAxis = -1;

	std::unique_ptr<tke::PlainRenderer> renderer;

	TransformerTool(tke::Framebuffer *_fb);
	virtual bool TransformerTool::leftDown(int x, int y) override;
	virtual void TransformerTool::mouseMove(int xDisp, int yDisp) override;
	virtual void show(tke::Camera *camera) override;
	tke::PlainRenderer::DrawData getDrawData(int draw_mode);
};
