#include <string>

#include "../src/core.h"

#include "editor.h"

#include <memory>

int main(int argc, char** argv)
{
	auto rrr = glm::rotate(90.f, glm::vec3(0.f, 1.f, 0.f));

	auto mm = glm::rotate(180.f, glm::vec3(0.f, 1.f, 0.f));

	auto vkTrans = glm::mat4(glm::vec4(1.f, 0.f, 0.f, 0.f), glm::vec4(0.f, -1.f, 0.f, 0.f),
		glm::vec4(0.f, 0.f, 1.f, 0.f), glm::vec4(0.f, 0.f, 0.f, 1.f));
	auto p = /*vkTrans * */glm::perspective(glm::radians(60.0f), 800.0f / 600.0f, 0.1f, 1000.0f);
	auto m = glm::lookAt(glm::vec3(0, 0, 1), glm::vec3(0, 0, -1), glm::vec3(0, 1, 0));
	auto pp = p * /*m * */glm::vec4(0.f, 1.f, -5.f, 1.f);

	tke::init(false, "../", 800, 600, 1280, 720, "TK Engine Editor", tke::WindowStyleHasFrameCanResize, false);
	setupEditor();
	tke::run();

	return 0;
}
