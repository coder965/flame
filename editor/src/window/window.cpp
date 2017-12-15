#include "../../../src/ui/ui.h"
#include "window.h"
#include "resource_explorer.h"
#include "scene_editor.h"

std::vector<WindowClass*> windowClasses;

Window::Window(WindowClass *_pClass)
	:pClass(_pClass)
{
	windows.push_back(std::move(std::unique_ptr<Window>(this)));
}

void Window::show()
{
	if (_need_focus)
	{
		ImGui::SetNextWindowFocus();
		_need_focus = false;
	}
	do_show();
}

std::vector<std::unique_ptr<Window>> windows;

void initWindow()
{
	windowClasses.push_back(&resourceExplorerClass);
}
