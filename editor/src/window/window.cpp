#include "../../../src/ui/ui.h"
#include "window.h"
#include "resource_explorer.h"
#include "scene_editor.h"

std::vector<IWindowClass*> windowClasses;

IWindow::IWindow(IWindowClass *_pClass)
	:pClass(_pClass)
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

void initWindow()
{
	windowClasses.push_back(&resourceExplorerClass);
}
