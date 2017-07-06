#pragma once

#include "../src/core.h"
#include "../src/scene.h"

#include "transformer_tool.h"

struct MonitorWidget
{
	tke::Scene *scene;
	tke::Model *model;

	tke::Image *image;
	tke::Framebuffer *fb_tool;
	tke::Framebuffer *fb_scene;

	tke::CommandBuffer *cb;
	tke::CommandBuffer *cb_wireframe;
	VkEvent renderFinished;
	VkEvent wireframe_renderFinished;

	TransformerTool *transformerTool;

	bool opened = true;

	MonitorWidget(tke::Model *_model);
	~MonitorWidget();
	void show();
};

extern MonitorWidget* monitorWidget;