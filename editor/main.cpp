#include <string>

#include "../src/core.h"

#include "editor.h"

#include <memory>

int main(int argc, char** argv)
{
	tke::init(false, "../", 1280, 720, 800, 600, "TK Engine Editor", tke::WindowStyleHasFrameCanResize, false);
	new EditorWindow;
	tke::run();

	return 0;
}
