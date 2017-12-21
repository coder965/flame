#include "../../ui/ui.h"
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
	if (first)
	{
		if (first_cx != 0 && first_cy != 0)
			ImGui::SetNextWindowSize(ImVec2(first_cx, first_cy));
	}
	do_show();

	first = false;
}

std::vector<std::unique_ptr<IWindow>> windows;
