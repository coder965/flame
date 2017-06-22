#pragma once

#include "../src/scene.h"

extern tke::Image *titleImage;

enum ItemType
{
	ItemTypeNull,
	ItemTypeObject,
	ItemTypeLight
};

struct SelectedItem : tke::Observer, tke::ObservedObject
{
	ItemType type = ItemTypeNull;
	tke::ObservedObject *ptr = nullptr;

	inline explicit operator bool() const noexcept
	{
		return type != ItemTypeNull;
	}

	inline tke::Object *toObject() const
	{
		return (tke::Object*)ptr;
	}

	void reset();
	void select(tke::Object *_obj);
	virtual void listen(void *sender, tke::NotificationType type, void *newData) override;
};

extern SelectedItem selectedItem;

struct EditorWindow : tke::Window
{
	EditorWindow();
	void on_view_gameExplorer();
	void on_view_output();
	void on_view_attributeWidget();
	void on_view_boneMotionWidget();
	virtual void renderEvent() override;
};

extern EditorWindow *mainWindow;