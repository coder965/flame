#include <string>

#include "../src/core.h"

#include "editor.h"

#include <memory>

int main(int argc, char** argv)
{
	tke::init(false, "../", 800, 600, 1280, 720, "TK Engine Editor", tke::WindowStyleHasFrameCanResize, false);
	setupEditor();
	tke::run();

	return 0;
}
