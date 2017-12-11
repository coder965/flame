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

	std::unique_ptr<tke::PlainRenderer> plain_renderer;

	bool enableRender = true;
	std::unique_ptr<tke::DeferredRenderer> defe_renderer;

	bool viewPhysx = false;
	std::unique_ptr<tke::OnceVertexBuffer> physx_vertex_buffer;
	std::unique_ptr<tke::LinesRenderer> lines_renderer;

	bool showSelectedWireframe = true;

	std::shared_ptr<tke::Framebuffer> fb_tool;
	std::unique_ptr<TransformerTool> transformerTool;

	bool follow = false;

	SceneEditor(std::shared_ptr<tke::Scene> _scene);
	virtual void show() override;
	virtual void save(tke::AttributeTreeNode *) override;
};
