#pragma once

#include "../../graphics/renderer.h"
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

	tke::Node *node = nullptr;
	Mode mode = ModeNull;
	tke::Node::Axis selectedAxis = tke::Node::AxisNull;

	std::unique_ptr<tke::PlainRenderer> renderer;

	TransformerTool(tke::Image *dst);
	virtual bool TransformerTool::leftDown(int x, int y) override;
	virtual void TransformerTool::mouseMove(int xDisp, int yDisp) override;
	virtual void show(tke::CameraComponent *camera) override;
	tke::PlainRenderer::DrawData getDrawData(int draw_mode);
};
