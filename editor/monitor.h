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

	VkCommandBuffer cmd;

	bool opened = true;

	MonitorWidget();

	~MonitorWidget();

	void show();
};

extern std::vector<MonitorWidget*> monitors;