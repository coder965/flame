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
					auto a = c->firstAttribute("opened");
					bool opened;
					a->get<bool>(&opened);
					if (opened)
						openGameExplorer();
				}
				else if (c->name == "MonitorWidget")
				{
					auto a0 = c->firstAttribute("opened");
					bool opened;
					a0->get<bool>(&opened);
					if (opened)
					{
						auto a1 = c->firstAttribute("model_filename");
						tke::Model *m = nullptr;
						for (auto _m : game.models)
						{
							if (_m->filename == a1->value)
							{
								m = _m->p;
								break;
							}
						}
						openMonitorWidget(m);
					}
				}
				else if (c->name == "AttributeWidget")
				{
					auto a = c->firstAttribute("opened");
					bool opened;
					a->get<bool>(&opened);
					if (opened)
						openAttributeWidget();
				}
			}
		}
	}
	tke::loadGuiDock("ui_dock.xml");

	cbs.push_back(ui->cb->v);
}

EditorWindow::~EditorWindow()
{
	tke::AttributeTree at("data");

	{
		auto n = new tke::AttributeTreeNode("GameExplorer");
		static bool opened = gameExplorer;
		n->attributes.push_back(new tke::Attribute("opened", &opened));
		at.children.push_back(n);
	}

	{
		auto n = new tke::AttributeTreeNode("MonitorWidget");
		static bool opened = monitorWidget;
		n->attributes.push_back(new tke::Attribute("opened", &opened));
		n->attributes.push_back(new tke::Attribute("model_filename", &monitorWidget->model->filename));
		at.children.push_back(n);
	}

	{
		auto n = new tke::AttributeTreeNode("AttributeWidget");
		static bool opened = attributeWidget;
		n->attributes.push_back(new tke::Attribute("opened", &opened));
		at.children.push_back(n);
	}

	at.saveXML("ui.xml");
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

void EditorWindow::openMonitorWidget(tke::Model *m)
{
	if (!monitorWidget)
		monitorWidget = new MonitorWidget(m);
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
			//cbs.push_back(monitorWidget->transformerTool->cb->v);
			ui->waitEvents.push_back(monitorWidget->renderFinished);
			//ui->waitEvents.push_back(monitorWidget->transformerTool->renderFinished);
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
	ui->waitEvents.clear();
}

EditorWindow *mainWindow = nullptr;