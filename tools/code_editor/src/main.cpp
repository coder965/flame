#include <flame/common/system.h>
#include <flame/common/image.h>
#include <flame/engine/core/core.h>
#include <flame/engine/core/application.h>
#include <flame/engine/graphics/texture.h>
#include <flame/engine/ui/ui.h>

struct App : flame::Application
{
	App() :
		Application(1280, 720, flame::WindowStyleFrame | flame::WindowStyleResizable, "code editor")
	{
	}

	virtual void on_render() override
	{
		ImGui::BeginOverlapWindow("main");

		ImGui::EndOverlapWindow();
	}
};

int main(int argc, char **args)
{
	flame::init("../", 1280, 720, 1, false, true);
	new App;
	flame::run();

	return 0;
}
