#pragma once

#include "../src/core.h"

#include "game.h"
#include "monitor.h"

extern tke::Image *titleImage;

enum ItemType
{
	ItemTypeNull,
	ItemTypeObject,
	ItemTypeLight
};

struct SelectedItem : tke::Observer, tke::ObservedObject
{
	ItemType selected_type = ItemTypeNull;
	void *selected_ptr = nullptr;

	void reset();

	void select(tke::Object *_obj);

	virtual void listen(void *sender, tke::NotificationType type, void *newData) override;
};

extern SelectedItem selectedItem;

struct EditorWindow : tke::Window
{
	EditorWindow();

	virtual void renderEvent() override;
};

extern EditorWindow *mainWindow;