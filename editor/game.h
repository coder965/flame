#pragma once

#include <vector>

#include "..\src\scene.h"

struct MonitorWidget;

struct Game
{
	std::vector<tke::Scene*> scenes;

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