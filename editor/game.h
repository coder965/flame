#pragma once

#include <vector>

struct MonitorWidget;

struct RendererEditorStruct
{
	std::string filename;
	MonitorWidget *monitor = nullptr;
};

struct ModelEditorStruct
{
	std::string filename;
	tke::Model *p = nullptr;
	MonitorWidget *monitor = nullptr;
};

struct Game
{
	std::vector<RendererEditorStruct*> renderers;
	std::vector<ModelEditorStruct*> models;

	void load();
	void save();
};

extern Game game;

struct GameExplorer
{
	bool opened = true;

	void show();
};

extern GameExplorer *gameExplorer;