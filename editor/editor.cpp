#include "../src/gui.h"

#include "editor.h"
#include "game.h"
#include "monitor.h"
#include "attribute.h"

LastWindowType lastWindowType = LastWindowTypeNull;
MonitorWidget *lastMonitor = nullptr;

tke::Image *titleImage = nullptr;

EditorWindow::EditorWindow()
	:Window(800, 600, "TK Engine Editor", true, true, WS_THICKFRAME)
{
	mainWindow = this;

	titleImage = tke::createImage("../misc/title.jpg", true);

	game.load();

	{
		tke::AttributeTree at("data", "ui.xml");
		if (at.good)
		{
			for (auto c : at.children)
			{
				if (c->name == "GameExplorer")
				{
					bool opened;
					c->firstAttribute("opened")->get(&opened);
					if (opened)
						openGameExplorer();
				}
				else if (c->name == "MonitorWidget")
				{
					bool opened;
					c->firstAttribute("opened")->get(&opened);
					if (opened)
					{
						auto a = c->firstAttribute("scene_filename");
						tke::Scene *s = nullptr;
						for (auto _s : game.scenes)
						{
							if (_s->filename == a->value)
							{
								s = _s;
								break;
							}
						}
						openMonitorWidget(s);
					}
				}
				else if (c->name == "AttributeWidget")
				{
					bool opened;
					c->firstAttribute("opened")->get(&opened);
					if (opened)
						openAttributeWidget();
				}
				else if (c->name == "object_creation_setting")
				{
					ocs.load_setting(c);
				}
			}
		}
	}

	tke::loadGuiDock("ui_dock.xml");
}

EditorWindow::~EditorWindow()
{
	{
		tke::AttributeTree at("data");
		{
			auto n = new tke::AttributeTreeNode("GameExplorer");
			n->addAttribute("opened", gameExplorer ? "true" : "false");
			at.children.push_back(n);
		}
		{
			auto n = new tke::AttributeTreeNode("MonitorWidget");
			n->addAttribute("opened", monitorWidget ? "true" : "false");
			n->addAttribute("scene_filename", monitorWidget->scene->filename);
			at.children.push_back(n);
		}
		{
			auto n = new tke::AttributeTreeNode("AttributeWidget");
			n->addAttribute("opened", attributeWidget ? "true" : "false");
			at.children.push_back(n);
		}
		{
			auto n = new tke::AttributeTreeNode("object_creation_setting");
			ocs.save_setting(n);
			at.children.push_back(n);
		}
		at.saveXML("ui.xml");
	}

	tke::saveGuiDock("ui_dock.xml");
}

void EditorWindow::openGameExplorer()
{
	if (!gameExplorer)
		gameExplorer = new GameExplorer;
}

void EditorWindow::openOutputWidget()
{

}

void EditorWindow::openMonitorWidget(tke::Scene *s)
{
	if (!monitorWidget)
		monitorWidget = new MonitorWidget(s);
}

void EditorWindow::openAttributeWidget()
{
	if (!attributeWidget)
		attributeWidget = new AttributeWidget;
}

void EditorWindow::renderEvent()
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
			openGameExplorer();
		}
		if (ImGui::MenuItem("Output"))
		{
		}
		if (ImGui::MenuItem("Attribute", nullptr, attributeWidget != nullptr))
		{
			openAttributeWidget();
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
	{
		gameExplorer->show();
		if (!gameExplorer->opened)
		{
			delete gameExplorer;
			gameExplorer = nullptr;
		}
	}

	if (attributeWidget)
	{
		attributeWidget->show();
		if (!attributeWidget->opened)
		{
			delete attributeWidget;
			attributeWidget = nullptr;
		}
	}

	if (monitorWidget)
	{
		monitorWidget->show();
		if (!monitorWidget->opened)
		{
			delete monitorWidget;
			monitorWidget = nullptr;
		}
		else
		{
			cbs.push_back(monitorWidget->cb->v);
			cbs.push_back(monitorWidget->cb_wireframe->v);
			cbs.push_back(monitorWidget->transformerTool->cb->v);
			ui->waitEvent = monitorWidget->renderFinished;
		}
	}

	ImGui::SetNextWindowPos(ImVec2(0, cy - ImGui::GetItemsLineHeightWithSpacing()));
	ImGui::Begin("status", nullptr, ImVec2(0, 0), 0.3f, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);
	ImGui::Text("FPS:%d", getFPS());
	ImGui::End();

	ui->end();

	cbs.push_back(ui->cb->v);

	endFrame();

	cbs.clear();
	ui->waitEvent = 0;
}

EditorWindow *mainWindow = nullptr;