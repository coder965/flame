#pragma once

#include "../src/core.h"

#include "game.h"
#include "monitor.h"

extern tke::Image *titleImage;

struct EditorWindow : tke::Window
{
	EditorWindow();

	virtual void renderEvent() override;
};

extern EditorWindow *mainWindow;