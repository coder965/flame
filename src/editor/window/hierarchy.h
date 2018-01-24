#pragma once

#include "../../ui/ui.h"

struct HierarchyWindow : tke::ui::Window
{
	HierarchyWindow();
	~HierarchyWindow();
	virtual void on_show() override;
};

extern HierarchyWindow *hierarchy_window;