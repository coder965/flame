#pragma once

#include "../src/core.h"
#include "../src/scene.h"

#include "transformer_tool.h"

struct MonitorWidget
{
	tke::Scene *scene;
	tke::Model *model;

	tke::Image *image;
	tke::Framebuffer *fb_one;
	tke::Framebuffer *fb_scene;

	tke::CommandBuffer *cb;
	VkEvent renderFinished;

	TransformerTool *transformerTool;

	bool opened = true;

	MonitorWidget(tke::Model *_model);
	~MonitorWidget();
	void makeCmd();
	void show();
};

extern std::vector<MonitorWidget*> monitors;