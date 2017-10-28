#include <string>

#include "../src/core.h"

#include "editor.h"

#include <memory>

int main(int argc, char** argv)
{
	tke::init(false, "../", 1280, 720, false);

	new EditorWindow;
	mainWindow->show();
	ShowWindow((HWND)mainWindow->hWnd, SW_SHOWMAXIMIZED);
	tke::run();

	return 0;
}
