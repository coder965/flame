#include <string>

#include "../src/core.h"

struct EditorWindow : tke::Window
{
	EditorWindow()
		:Window(800, 600, "", true, true, WS_THICKFRAME)
	{

	}

	virtual void renderEvent() override
	{
		beginFrame();

		ui->begin(false, true);
		ImGui::SetNextWindowPos(ImVec2(0, cy - ImGui::GetItemsLineHeightWithSpacing()));
		ImGui::Begin("status", nullptr, ImVec2(0, 0), 0.3f, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);
		ImGui::Text("FPS:%d", getFPS());
		ImGui::End();
		ui->end();

		VkCommandBuffer cmds[1] = { ui->cmd };
		tke::graphicsQueue.submitFence(imageAvailable, 1, cmds, frameDone);

		endFrame();
	}
};

int main(int argc, char** argv)
{
	tke::init("../", 800, 600);

	auto w = new EditorWindow;
	w->addToList();
	tke::run();

	return 0;
}

