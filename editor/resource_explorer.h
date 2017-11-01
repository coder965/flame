#pragma once

#include <vector>

struct MonitorWidget;

void load_resource();
void save_resource();

struct ResourceExplorer
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

extern ResourceExplorer *resourceExplorer;