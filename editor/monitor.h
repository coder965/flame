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
	tke::Framebuffer *fb_image;
	tke::Framebuffer *fb_tool;

	tke::VertexBuffer *physxBuffer = nullptr;

	tke::CommandBuffer *cb;
	tke::CommandBuffer *cb_physx;
	tke::CommandBuffer *cb_wireframe;
	tke::DescriptorSet *ds_wireframe;

	VkEvent scene_renderFinished;
	VkEvent physx_renderFinished;
	VkEvent wireframe_renderFinished;
	VkEvent renderFinished;

	TransformerTool *transformerTool;

	bool opened = true;

	MonitorWidget(tke::Scene *scene);
	~MonitorWidget();
	void show();
};

extern MonitorWidget* monitorWidget;