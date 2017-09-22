#include <string>

#include "../src/core.h"
#include "../src/window.h"
#include "../src/ui/ui.h"

#include <memory>

struct MyWindow : tke::Window
{
	MyWindow()
		:Window(800, 600, "Count Down", tke::WindowStyleHasFrameNoResize, true)
	{

	}

	virtual ~MyWindow() override
	{

	}

	virtual void renderEvent() override
	{
		beginFrame(true);

		ImGui::SetNextWindowPos(ImVec2(0, cy - ImGui::GetItemsLineHeightWithSpacing()));
		ImGui::Begin("status", nullptr, ImVec2(0, 0), 0.3f, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);
		ImGui::Text("FPS:%d", getFPS());
		ImGui::End();

		endFrame();
	}
};


int main(int argc, char** argv)
{
	tke::init(false, "../", 800, 600, true);

	auto window = new MyWindow;
	window->show();
	tke::run();

	return 0;
}
