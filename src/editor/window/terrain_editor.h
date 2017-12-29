#pragma once

#include "../../graphics/display_layer.h"
#include "../../graphics/buffer.h"
#include "../../graphics/image.h"
#include "../../graphics/framebuffer.h"
#include "../../graphics/renderer.h"
#include "../../model/model.h"
#include "../../entity/camera.h"
#include "../../entity/terrain.h"

#include "window.h"

struct TerrainEditor : IWindow
{
	std::shared_ptr<tke::Terrain> terrain;

	tke::DisplayLayer layer;

	std::unique_ptr<float[]> height_array;
	std::unique_ptr<tke::UniformBuffer> height_array_buffer;

	std::vector<tke::ModelVertex> vertexs;
	std::vector<int> indices;

	std::unique_ptr<tke::VertexBuffer> vertex_buffer;
	std::unique_ptr<tke::IndexBuffer> index_buffer;

	tke::Camera camera;
	std::unique_ptr<tke::PlainRenderer> renderer;

	int block_count = 64;

	TerrainEditor();
	virtual void do_show() override;
	void create_vertex();
};
