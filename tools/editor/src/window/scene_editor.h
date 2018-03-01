#pragma once

#include <flame/utils/filesystem.h>
#include <flame/entity/scene.h>
#include <flame/graphics/display_layer.h>
#include <flame/graphics/renderer.h>
#include <flame/ui/ui.h>
#include "../tool/transformer_tool.h"

struct SceneEditor : flame::ui::Window
{
	flame::Node *camera_node;
	flame::CameraComponent *camera;

	flame::Scene *scene;

	flame::DisplayLayer layer;

	std::unique_ptr<flame::PlainRenderer> plain_renderer;

	bool enableRender = true;
	std::unique_ptr<flame::DeferredRenderer> defe_renderer;

	bool viewPhysx = false;
	std::unique_ptr<flame::Buffer> physx_vertex_buffer;
	std::unique_ptr<flame::LinesRenderer> lines_renderer;

	bool showSelectedWireframe = true;

	Tool *curr_tool;
	std::unique_ptr<TransformerTool> transformerTool;

	SceneEditor(flame::Scene *_scene);
	~SceneEditor();
	void on_file_menu();
	void on_menu_bar();
	void on_toolbar();
	virtual void on_show() override;
	void save(flame::XMLNode *);

	void on_delete();
};

extern SceneEditor *scene_editor;