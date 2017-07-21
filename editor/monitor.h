#pragma once

#include "../src/core.h"
#include "../src/entity.h"

#include "select.h"
#include "transformer_tool.h"

struct MonitorWidget
{
	enum Mode
	{
		ModeScene,
		ModeModel
	};
	Mode mode;

	std::vector<VkCommandBuffer> cbs;
	VkEvent renderFinished;

	bool opened = true;

	MonitorWidget();
	virtual ~MonitorWidget();
	virtual void show() = 0;
};

struct SceneMonitorWidget : MonitorWidget
{
	tke::Image *image;
	tke::Framebuffer *fb_image;

	tke::Scene *scene;
	bool showSelectedWireframe = true;
	bool viewPhysx = false;
	tke::Object *last_obj = nullptr;
	tke::Framebuffer *fb_scene;
	tke::CommandBuffer *cb_scene;
	VkEvent scene_renderFinished;

	SelectedItem selectedItem;

	tke::OnceVertexBuffer *physxBuffer = nullptr;
	tke::CommandBuffer *cb_physx;
	VkEvent physx_renderFinished;

	tke::CommandBuffer *cb_wireframe;
	tke::DescriptorSet *ds_wireframe_anim;
	VkEvent wireframe_renderFinished;

	tke::Framebuffer *fb_tool;
	TransformerTool *transformerTool;

	SceneMonitorWidget(tke::Scene *_scene);
	virtual ~SceneMonitorWidget() override;
	virtual void show() override;
};

struct ModelMonitorWidget : MonitorWidget
{
	tke::Image *image;
	tke::Framebuffer *fb_image;

	tke::Model *model;
	bool showController = false;
	bool showEyePosition = false;
	tke::Framebuffer *fb_model;
	tke::Camera camera;
	tke::AnimationComponent *animComp = nullptr;
	tke::CommandBuffer *cb;
	tke::DescriptorSet *ds_anim;
	VkEvent model_renderFinished;

	tke::CommandBuffer *cb_wireframe;

	ModelMonitorWidget(tke::Model *_model);
	virtual ~ModelMonitorWidget() override;
	virtual void show() override;
};

extern std::vector<MonitorWidget*> monitorWidgets;