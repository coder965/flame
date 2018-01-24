#pragma once

#include "../../graphics/display_layer.h"
#include "../../graphics/framebuffer.h"
#include "../../graphics/renderer.h"
#include "../../model/model.h"
#include "../../entity/terrain.h"
#include "../../entity/camera.h"

#include "../../ui/ui.h"

struct ModelEditor : tke::ui::Window
{
	std::shared_ptr<tke::Model> model;
	tke::PlainRenderer::DrawData draw_data;

	tke::DisplayLayer layer;

	tke::CameraComponent camera;
	std::unique_ptr<tke::PlainRenderer> renderer;

	ModelEditor(std::shared_ptr<tke::Model> _model);
	virtual void on_show() override;
};
