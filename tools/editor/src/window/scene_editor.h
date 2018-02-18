#pragma once

#include <flame/utils/file.h>
#include <flame/entity/scene.h>
#include <flame/graphics/display_layer.h>
#include <flame/graphics/renderer.h>
#include <flame/ui/ui.h>
#include "../tool/transformer_tool.h"

struct SceneEditor : tke::ui::Window
{
	tke::Node *camera_node;
	tke::CameraComponent *camera;

	tke::Scene *scene;

	tke::DisplayLayer layer;

	std::unique_ptr<tke::PlainRenderer> plain_renderer;

	bool enableRender = true;
	std::unique_ptr<tke::DeferredRenderer> defe_renderer;

	bool viewPhysx = false;
	std::unique_ptr<tke::Buffer> physx_vertex_buffer;
	std::unique_ptr<tke::LinesRenderer> lines_renderer;

	bool showSelectedWireframe = true;

	Tool *curr_tool;
	std::unique_ptr<TransformerTool> transformerTool;

	SceneEditor(tke::Scene *_scene);
	~SceneEditor();
	void on_file_menu();
	void on_menu_bar();
	void on_toolbar();
	virtual void on_show() override;
	void save(tke::XMLNode *);

	void on_delete();
};

extern SceneEditor *scene_editor;