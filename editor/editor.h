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
	virtual ~EditorWindow() override;
	void openGameExplorer();
	void openOutputWidget();
	void openMonitorWidget(const std::string &renderer_filename, tke::Model *m);
	void openAttributeWidget();
	void openBoneMotionWidget();
	virtual void renderEvent() override;
	void saveUi(const std::string &filename);
	void loadUi(const std::string &filename);
};

extern EditorWindow *mainWindow;