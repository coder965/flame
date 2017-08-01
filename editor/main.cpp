#include <string>

#include "../src/core.h"
#include "../src/entity.h"

#include "editor.h"

struct Fuck
{
	std::string id;
	float *s = nullptr;

	~Fuck()
	{
		delete []s;
	}
};

#include <memory>

int main(int argc, char** argv)
{
	tke::init("../", 1280, 720);

	new EditorWindow;
	mainWindow->show();
	ShowWindow((HWND)mainWindow->hWnd, SW_SHOWMAXIMIZED);
	tke::run();

	return 0;
}
