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
	{
		std::vector<std::unique_ptr<Fuck>> f;

		auto a = std::make_unique<Fuck>(); a->id = "sdfsaf df sdf df dfas df fsadf sad"; a->s = new float[24];
		auto b = std::make_unique<Fuck>(); b->id = "dsf sdfbfijokng ojfbg jfbsg s"; b->s = new float[36];

		f.push_back(std::move(a));
	}

	tke::init("../", 1280, 720);

	new EditorWindow;
	mainWindow->show();
	ShowWindow((HWND)mainWindow->hWnd, SW_SHOWMAXIMIZED);
	tke::run();

	return 0;
}
