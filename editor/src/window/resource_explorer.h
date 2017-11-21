#pragma once

#include <vector>
#include "window.h"

struct ResourceExplorerClass : WindowClass
{
	virtual std::string getName() override;
	virtual Window *load(tke::AttributeTreeNode *n) override;
};

extern ResourceExplorerClass resourceExplorerClass;

void load_resource();
void save_resource();

struct ResourceExplorer : Window
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

	ResourceExplorer();
	virtual void show() override;
};

extern ResourceExplorer *resourceExplorer;