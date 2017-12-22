#pragma once

#include "../../core.h"
#include "../../render/display_layer.h"
#include "../../render/renderer.h"
#include "window.h"
#include "../select.h"
#include "../tool/transformer_tool.h"

struct SceneEditor
{
	std::shared_ptr<tke::Scene> scene;

	tke::DisplayLayer layer;

	std::unique_ptr<tke::PlainRenderer> plain_renderer;

	bool enableRender = true;
	std::unique_ptr<tke::DeferredRenderer> defe_renderer;

	bool viewPhysx = false;
	std::unique_ptr<tke::ImmediateVertexBuffer> physx_vertex_buffer;
	std::unique_ptr<tke::LinesRenderer> lines_renderer;

	bool showSelectedWireframe = true;

	std::shared_ptr<tke::Framebuffer> fb_tool;
	std::unique_ptr<TransformerTool> transformerTool;

	bool follow = false;

	SceneEditor(std::shared_ptr<tke::Scene> _scene);
	void on_file_menu();
	void on_menu_bar();
	void do_show();
	void save(tke::AttributeTreeNode *);
};

extern std::unique_ptr<SceneEditor> scene_editor;