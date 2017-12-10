#pragma once

#include "../../../src/render/display_layer.h"
#include "../../../src/render/framebuffer.h"
#include "../../../src/render/renderer.h"
#include "../../../src/entity/terrain.h"
#include "../../../src/model/model.h"

#include "window.h"

struct ModelEditorClass : WindowClass
{
	virtual std::string getName() override;
	virtual Window *load(tke::AttributeTreeNode *n) override;
};

extern ModelEditorClass modelEditorClass;

struct ModelEditor : Window
{
	std::shared_ptr<tke::Model> model;

	tke::DisplayLayer layer;

	std::unique_ptr<tke::PlainRenderer> renderer;

	ModelEditor(std::shared_ptr<tke::Model> _model);
	virtual void show() override;
};
