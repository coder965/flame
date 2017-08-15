#include <string>

#include "../src/core.h"
#include "../src/entity.h"

#include "editor.h"

#include <memory>

int main(int argc, char** argv)
{
	auto a = glm::mat4(1.f);

	tke::init("../", 1280, 720);

	new EditorWindow;
	mainWindow->show();
	ShowWindow((HWND)mainWindow->hWnd, SW_SHOWMAXIMIZED);
	tke::run();

	return 0;
}
