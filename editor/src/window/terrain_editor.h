#pragma once

#include "../../../src/render/display_layer.h"
#include "../../../src/render/framebuffer.h"
#include "../../../src/entity/terrain.h"

#include "window.h"

struct TerrainEditorClass : WindowClass
{
	virtual std::string getName() override;
	virtual Window *load(tke::AttributeTreeNode *n) override;
};

extern TerrainEditorClass terrainEditorClass;

struct TerrainEditor : Window
{
	std::shared_ptr<tke::Terrain> terrain;

	tke::DisplayLayer layer;

	int tess = 100;

	TerrainEditor();
	virtual void show() override;
};
