#pragma once

#include "../src/core.h"
#include "../src/entity.h"

#include "select.h"
#include "transformer_tool.h"

struct MonitorWidget
{
	tke::Image *image;
	tke::Framebuffer *fb_image;

	tke::Scene *scene = nullptr;
	tke::Framebuffer *fb_scene;
	tke::CommandBuffer *cb_scene;
	VkEvent scene_renderFinished;

	SelectedItem selectedItem;

	tke::OnceVertexBuffer *physxBuffer = nullptr;
	tke::CommandBuffer *cb_physx;
	VkEvent physx_renderFinished;

	tke::CommandBuffer *cb_wireframe;
	tke::DescriptorSet *ds_wireframe;
	VkEvent wireframe_renderFinished;

	tke::Framebuffer *fb_tool;
	TransformerTool *transformerTool;

	VkEvent renderFinished;

	bool opened = true;

	MonitorWidget(tke::Scene *scene);
	~MonitorWidget();
	void show();
};

extern std::vector<MonitorWidget*> monitorWidgets;