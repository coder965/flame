#pragma once

#include "../../../src/render/display_layer.h"
#include "../../../src/render/framebuffer.h"
#include "../../../src/render/renderer.h"
#include "../../../src/entity/terrain.h"
#include "../../../src/model/model.h"

#include "window.h"

struct ModelEditor : IWindow
{
	std::shared_ptr<tke::Model> model;
	tke::PlainRenderer::DrawData draw_data;

	tke::DisplayLayer layer;

	tke::Camera camera;
	std::unique_ptr<tke::PlainRenderer> renderer;

	ModelEditor(std::shared_ptr<tke::Model> _model);
	virtual void do_show() override;
};
