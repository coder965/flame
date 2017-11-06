#pragma once

struct TerrainEditor
{
	int tess = 100;

	bool opened = true;

	void show();
};

extern TerrainEditor *terrainEditor;
