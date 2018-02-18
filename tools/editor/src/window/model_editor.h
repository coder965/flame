#pragma once

#include <flame/graphics/display_layer.h>
#include <flame/graphics/renderer.h>
#include <flame/model/model.h>
#include <flame/entity/camera.h>
#include <flame/ui/ui.h>

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
