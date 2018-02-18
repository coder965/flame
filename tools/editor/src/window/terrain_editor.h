#pragma once

#include <flame/graphics/display_layer.h>
#include <flame/graphics/buffer.h>
#include <flame/graphics/texture.h>
#include <flame/graphics/framebuffer.h>
#include <flame/graphics/renderer.h>
#include <flame/model/model.h>
#include <flame/entity/camera.h>
#include <flame/entity/terrain.h>
#include <flame/ui/ui.h>

struct TerrainEditor : tke::ui::Window
{
	tke::Node *camera_node;
	tke::CameraComponent *camera;

	tke::Node *terrain_node;
	tke::TerrainComponent* terrain;

	tke::DisplayLayer layer;

	std::unique_ptr<float[]> height_array;
	std::unique_ptr<tke::Buffer> height_array_buffer;

	std::vector<tke::ModelVertex> vertexs;
	std::vector<int> indices;

	std::unique_ptr<tke::Buffer> vertex_buffer;
	std::unique_ptr<tke::Buffer> index_buffer;

	std::unique_ptr<tke::PlainRenderer> renderer;

	int block_count = 64;

	TerrainEditor();
	~TerrainEditor();
	virtual void on_show() override;
	void create_vertex();
};
