#pragma once

#include "../../../src/core.h"
#include "../../../src/render/display_layer.h"
#include "../../../src/render/renderer.h"
#include "window.h"
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

	tke::DisplayLayer layer;

	std::shared_ptr<tke::Framebuffer> fb_scene;
	VkEvent scene_renderFinished;

	bool viewPhysx = false;
	tke::OnceVertexBuffer *physxBuffer = nullptr;
	tke::CommandBuffer *cb_physx;
	VkEvent physx_renderFinished;

	bool showSelectedWireframe = true;
	std::unique_ptr<tke::WireframeRenderer> wireframe_renderer;

	std::shared_ptr<tke::Framebuffer> fb_tool;
	TransformerTool *transformerTool;

	VkEvent renderFinished;

	bool follow = false;

	SceneEditor(std::shared_ptr<tke::Scene> _scene);
	~SceneEditor();
	virtual void show() override;
	virtual void save(tke::AttributeTreeNode *) override;
};
