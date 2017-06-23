#include <string>

#include "../src/core.h"
#include "../src/scene.h"

#include "editor.h"

int main(int argc, char** argv)
{
	tke::init("../", 1280, 720);

	new EditorWindow;
	mainWindow->addToList();
	ShowWindow(mainWindow->hWnd, SW_SHOWMAXIMIZED);
	tke::run();

	return 0;
}
