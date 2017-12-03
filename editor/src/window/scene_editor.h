#pragma once

#include "window.h"
#include "../../../src/core.h"
#include "../select.h"
#include "../tool/transformer_tool.h"

struct SceneEditorClass : WindowClass
{
	virtual std::string getName() override;
	virtual Window *load(tke::AttributeTreeNode *n) override;
};

extern SceneEditorClass sceneEditorClass;

struct SceneEditor : Window
{
	std::shared_ptr<tke::Scene> scene;

	std::shared_ptr<tke::Image> image;
	std::shared_ptr<tke::Framebuffer> fb_image;

	tke::Object *last_obj = nullptr;
	std::shared_ptr<tke::Framebuffer> fb_scene;
	VkEvent scene_renderFinished;

	bool viewPhysx = false;
	tke::OnceVertexBuffer *physxBuffer = nullptr;
	tke::CommandBuffer *cb_physx;
	VkEvent physx_renderFinished;

	bool showSelectedWireframe = true;
	tke::CommandBuffer *cb_wireframe;
	tke::DescriptorSet *ds_wireframe_anim;
	VkEvent wireframe_renderFinished;

	std::shared_ptr<tke::Framebuffer> fb_tool;
	TransformerTool *transformerTool;

	std::vector<VkCommandBuffer> cbs;
	VkEvent renderFinished;

	bool follow = false;

	SceneEditor(std::shared_ptr<tke::Scene> _scene);
	~SceneEditor();
	virtual void show() override;
	virtual void save(tke::AttributeTreeNode *) override;
};
