#pragma once

#include "../../ui/ui.h"

struct InspectorWindow : tke::ui::Window
{
	InspectorWindow();
	~InspectorWindow();
	virtual void on_show() override;
};

extern InspectorWindow *inspector_window;