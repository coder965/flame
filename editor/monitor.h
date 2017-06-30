#pragma once

#include "../src/core.h"
#include "../src/scene.h"

struct MonitorWidget
{
	std::string renderer_filename;
	tke::Renderer *renderer;
	tke::Scene *scene;
	tke::Model *model;
	tke::Image *image;

	VkEvent renderFinished;

	tke::CommandBuffer *cb;

	bool opened = true;

	MonitorWidget(const std::string _renderer_filename, tke::Model *_model);
	~MonitorWidget();
	void makeCmd();
	void show();
};

extern std::vector<MonitorWidget*> monitors;