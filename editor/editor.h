#pragma once

#include "../src/entity.h"

#include "game.h"
#include "monitor.h"

enum LastWindowType
{
	LastWindowTypeNull,
	LastWindowTypeGameExplorer,
	LastWindowTypeMonitor
};
extern LastWindowType lastWindowType;

extern tke::Image *titleImage;

struct EditorWindow : tke::Window
{
	EditorWindow();
	virtual ~EditorWindow() override;
	void openGameExplorer();
	void openOutputWidget();
	void openMonitorWidget(tke::Scene *s);
	void openAttributeWidget();
	virtual void renderEvent() override;
};

extern EditorWindow *mainWindow;