#pragma once

#include <vector>

struct MonitorWidget;

struct Game
{
	void load();
	void save();
};

extern Game game;

struct GameExplorer
{
	enum LastItemType
	{
		lastItemTypeNull,
		lastItemTypeTexture,
		lastItemTypeModel,
		lastItemTypeAnimation,
		lastItemTypeScene
	};

	LastItemType lastItemType = lastItemTypeNull;
	int itemIndex = -1;
	bool opened = true;

	void show();
};

extern GameExplorer *gameExplorer;