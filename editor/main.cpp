#include <string>

#include "../src/core.h"
#include "../src/scene.h"

struct EditorWindow;
EditorWindow *mainWindow = nullptr;

struct GameExplorer;
GameExplorer *gameExplorer = nullptr;

struct GameExplorer
{
	bool opened = true;

	void show()
	{
		ImGui::Begin("Game Explorer", &opened);
		if (ImGui::TreeNode("Renderers"))
		{
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Scenes"))
		{
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Models"))
		{
			ImGui::TreePop();
		}
		ImGui::End();

		if (!opened)
		{
			gameExplorer = nullptr;
			delete this;
		}
	}
};

struct MonitorWidget;
std::vector<MonitorWidget*> monitors;

struct MonitorWidget
{
	std::string renderer_filename;
	tke::Renderer *renderer;
	tke::Scene *scene;
	tke::Model *model;
	tke::Image *image;

	VkEvent renderFinished;

	VkCommandBuffer cmd;

	bool opened = true;

	MonitorWidget()
	{
		renderer = new tke::Renderer;
		renderer->loadXML(renderer_filename);
		scene = new tke::Scene;

		image = new tke::Image(800, 600, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
		renderer->resource.setImage(image, "Window.Image");
		tke::addGuiImage(image);
		tke::ShaderMacro macro;
		macro.pipelineName = "Deferred.Pipeline";
		macro.stageType = tke::StageType::frag;
		macro.value = "#define USE_PBR\n";
		renderer->resource.shaderMacros.push_back(macro);

		renderer->setup();

		tke::setMasterRenderer(renderer);
		scene->setRenderer(renderer);

		tke::needRedraw = true;
		tke::needUpdateTexture = true;
		scene->needUpdateSky = true;

		renderFinished = tke::createEvent();
		mainWindow->addEvent(renderFinished);

		cmd = tke::commandPool.allocate();
		tke::beginCommandBuffer(cmd);
		renderer->execute(cmd);
		vkCmdSetEvent(cmd, renderFinished, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
		vkEndCommandBuffer(cmd);
	}

	~MonitorWidget()
	{
		mainWindow->removeEvent(renderFinished);
		tke::destroyEvent(renderFinished);
	}

	void show()
	{
		ImGui::Begin("Monitor", &opened);

		if (!opened)
		{
			for (auto it = monitors.begin(); it != monitors.end(); it++)
			{
				if (*it == this)
				{
					monitors.erase(it);
					break;
				}
			}
			delete this;
		}
	}
};

struct EditorWindow : tke::Window
{
	EditorWindow()
		:Window(800, 600, "TK Engine Editor", true, true, WS_THICKFRAME)
	{
	}

	virtual void renderEvent() override
	{
		beginFrame();

		ui->begin(true);

		ImGui::BeginMainMenuBar();
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::BeginMenu("New"))
			{
				if (ImGui::MenuItem("Game"))
				{
				}
				if (ImGui::MenuItem("Renderer"))
				{
				}
				if (ImGui::MenuItem("Scene"))
				{
				}
				ImGui::EndMenu();
			}
			if (ImGui::MenuItem("Save Selected Item"))
			{
			}
			if (ImGui::MenuItem("Save Selected Item As"))
			{
			}
			if (ImGui::MenuItem("Save All"))
			{
			}
			if (ImGui::MenuItem("Open In Explorer"))
			{
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Edit"))
		{
			if (ImGui::MenuItem("Undo"))
			{
			}
			if (ImGui::MenuItem("Redo"))
			{
			}
			if (ImGui::MenuItem("Cut"))
			{
			}
			if (ImGui::MenuItem("Copy"))
			{
			}
			if (ImGui::MenuItem("Paste"))
			{
			}
			if (ImGui::MenuItem("Remove"))
			{
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("View"))
		{
			if (ImGui::MenuItem("Game Explorer", nullptr, gameExplorer != nullptr))
			{
				if (!gameExplorer)
					gameExplorer = new GameExplorer;
			}
			if (ImGui::MenuItem("Output"))
			{
			}
			if (ImGui::MenuItem("Attribute"))
			{
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Build"))
		{
			if (ImGui::MenuItem("Build"))
			{
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Debug"))
		{
			if (ImGui::MenuItem("Update Changes"))
			{
			}
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();

		if (gameExplorer)
			gameExplorer->show();

		ImGui::SetNextWindowPos(ImVec2(0, cy - ImGui::GetItemsLineHeightWithSpacing()));
		ImGui::Begin("status", nullptr, ImVec2(0, 0), 0.3f, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);
		ImGui::Text("FPS:%d", getFPS());
		ImGui::End();

		ui->end();

		std::vector<VkCommandBuffer> cmds;
		for (auto m : monitors)
			cmds.push_back(m->cmd);
		cmds.push_back(ui->cmd);

		tke::graphicsQueue.submitFence(imageAvailable, cmds.size(), cmds.data(), frameDone);

		endFrame();
	}
};

int main(int argc, char** argv)
{
	tke::init("../", 800, 600);

	mainWindow = new EditorWindow;
	mainWindow->addToList();
	ShowWindow(mainWindow->hWnd, SW_SHOWMAXIMIZED);
	tke::run();

	return 0;
}

