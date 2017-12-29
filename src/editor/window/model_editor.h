#pragma once

#include "../../graphics/display_layer.h"
#include "../../graphics/framebuffer.h"
#include "../../graphics/renderer.h"
#include "../../model/model.h"
#include "../../entity/terrain.h"
#include "../../entity/camera.h"

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
