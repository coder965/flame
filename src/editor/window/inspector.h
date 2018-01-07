#pragma once

#include "window.h"

struct InspectorWindow : IWindow
{
	~InspectorWindow();
	virtual void do_show() override;
};

extern InspectorWindow *inspector_window;