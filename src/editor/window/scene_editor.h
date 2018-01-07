#pragma once

#include "../../file_utils.h"
#include "../../entity/scene.h"
#include "../../graphics/display_layer.h"
#include "../../graphics/renderer.h"
#include "../tool/transformer_tool.h"

struct SceneEditor
{
	std::shared_ptr<tke::Scene> scene;
	std::weak_ptr<tke::Node> selected;

	tke::DisplayLayer layer;

	std::unique_ptr<tke::PlainRenderer> plain_renderer;

	bool enableRender = true;
	std::unique_ptr<tke::DeferredRenderer> defe_renderer;

	bool viewPhysx = false;
	std::unique_ptr<tke::ImmediateVertexBuffer> physx_vertex_buffer;
	std::unique_ptr<tke::LinesRenderer> lines_renderer;

	bool showSelectedWireframe = true;

	std::unique_ptr<TransformerTool> transformerTool;

	bool follow = false;

	SceneEditor(std::shared_ptr<tke::Scene> _scene);
	void on_file_menu();
	void on_menu_bar();
	void do_show();
	void save(tke::XMLNode *);
};

extern std::unique_ptr<SceneEditor> scene_editor;