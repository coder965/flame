#pragma once

#include <flame/graphics/display_layer.h>
#include <flame/graphics/renderer.h>
#include <flame/entity/model.h>
#include <flame/entity/camera.h>
#include <flame/ui/ui.h>

struct ModelEditor : flame::ui::Window
{
	std::shared_ptr<flame::Model> model;
	flame::PlainRenderer::DrawData draw_data;

	flame::DisplayLayer layer;

	flame::CameraComponent camera;
	std::unique_ptr<flame::PlainRenderer> renderer;

	ModelEditor(std::shared_ptr<flame::Model> _model);
	virtual void on_show() override;
};
