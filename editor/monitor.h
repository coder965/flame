#pragma once

#include "../src/core.h"
#include "../src/scene.h"

#include "transformer_tool.h"

struct MonitorWidget
{
	std::string renderer_filename;
	tke::Renderer *renderer;
	tke::Scene *scene;
	tke::Model *model;

	tke::Image *image;
	tke::Framebuffer *fb_one;
	tke::Framebuffer *fb_scene;

	tke::CommandBuffer *cb;
	VkEvent renderFinished;

	TransformerTool *transformerTool;

	bool opened = true;

	MonitorWidget(const std::string _renderer_filename, tke::Model *_model);
	~MonitorWidget();
	void makeCmd();
	void show();
};

extern std::vector<MonitorWidget*> monitors;