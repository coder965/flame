#include "../../../src/ui/ui.h"
#include "window.h"
#include "resource_explorer.h"
#include "scene_editor.h"

IWindow::IWindow()
{
	windows.push_back(std::move(std::unique_ptr<IWindow>(this)));
}

void IWindow::show()
{
	if (_need_focus)
	{
		ImGui::SetNextWindowFocus();
		_need_focus = false;
	}
	do_show();
}

std::vector<std::unique_ptr<IWindow>> windows;
