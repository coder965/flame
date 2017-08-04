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
	auto d = tke::LinearDepthPerspective(0.93594f, 0.1f, 1000.f);

	auto matrix = glm::mat4(1.f, 0.f, 0.f, 0.f,
		0.f, 1.f, 0.f, 0.f,
		0.f, 0.f, 0.5f, 0.f,
		0.f, 0.f, 0.5f, 1.f) *
		glm::ortho(-1.f, 1.f, -1.f, 1.f, TKE_NEAR, TKE_FAR);

	auto p0 = matrix * glm::vec4(0.f, 0.f, -100.f, 1.f);
	auto p1 = matrix * glm::vec4(0.f, 0.f, -101.f, 1.f);

	auto e0 = glm::exp(p0.z * 300.f);
	auto e1 = glm::exp(p1.z * 300.f);

	auto t = e0 / e1;

	auto cut = 1;

	tke::init("../", 1280, 720);

	new EditorWindow;
	mainWindow->show();
	ShowWindow((HWND)mainWindow->hWnd, SW_SHOWMAXIMIZED);
	tke::run();

	return 0;
}
