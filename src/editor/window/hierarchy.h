#pragma once

#include "window.h"

struct HierarchyWindow : IWindow
{
	~HierarchyWindow();
	virtual void do_show() override;
};

extern HierarchyWindow *hierarchy_window;