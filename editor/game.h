#pragma once

#include <vector>

struct MonitorWidget;

struct ModelEditorStruct
{
	std::string filename;
	tke::Model *p = nullptr;
	MonitorWidget *monitor = nullptr;
};

struct Game
{
	std::vector<ModelEditorStruct*> models;

	void load();
	void save();
};

extern Game game;

struct GameExplorer
{
	enum LastItemType
	{
		lastItemTypeNull,
		lastItemTypeModel,
		lastItemTypeScene
	};

	LastItemType lastItemType = lastItemTypeNull;
	int itemIndex = -1;
	bool opened = true;

	void show();
};

extern GameExplorer *gameExplorer;