#pragma once

#include "../../../src/render/display_layer.h"
#include "../../../src/render/framebuffer.h"
#include "../../../src/render/renderer.h"
#include "../../../src/entity/terrain.h"

#include "window.h"

struct TerrainEditor : IWindow
{
	std::shared_ptr<tke::Terrain> terrain;

	tke::DisplayLayer layer;

	std::vector<tke::Vertex> vertexs;
	std::vector<int> indices;

	std::unique_ptr<tke::VertexBuffer> vertex_buffer;
	std::unique_ptr<tke::IndexBuffer> index_buffer;

	tke::Camera camera;
	std::unique_ptr<tke::PlainRenderer> renderer;

	int block_count = 64;

	TerrainEditor();
	virtual void do_show() override;
	void create_vertex(bool first);
};
