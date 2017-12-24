#pragma once

#include "../../entity/scene.h"
#include "window.h"

struct EntityWindow : IWindow
{
	tke::Scene *scene;

	EntityWindow(tke::Scene *_scene);
	~EntityWindow();
	virtual void do_show() override;
};

extern EntityWindow *entity_window;