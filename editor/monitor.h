#pragma once

#include "../src/core.h"
#include "../src/entity.h"

#include "select.h"
#include "transformer_tool.h"

struct MonitorWidget
{
	tke::Scene *scene;

	SelectedItem selectedItem;

	tke::Image *image;
	tke::Framebuffer *fb_scene;
	tke::Framebuffer *fb_wireframe;
	tke::Framebuffer *fb_tool;

	tke::CommandBuffer *cb;
	tke::CommandBuffer *cb_wireframe;
	tke::DescriptorSet *ds_wireframe;
	VkEvent scene_renderFinished;
	VkEvent wireframe_renderFinished;
	VkEvent renderFinished;

	TransformerTool *transformerTool;

	bool opened = true;

	MonitorWidget(tke::Scene *scene);
	~MonitorWidget();
	void show();
};

extern MonitorWidget* monitorWidget;