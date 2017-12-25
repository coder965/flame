#pragma once

#include "../../entity/scene.h"
#include "window.h"

struct EntityWindow : IWindow
{
	~EntityWindow();
	virtual void do_show() override;
};

extern EntityWindow *entity_window;