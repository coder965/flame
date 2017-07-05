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
				else if (c->name == "Monitor")
				{
					auto a = c->firstAttribute("model_filename");
					tke::Model *m = nullptr;
					for (auto _m : game.models)
					{
						if (_m->filename == a->value)
						{
							m = _m->p;
							break;
						}
					}
					openMonitorWidget(m);
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
}

EditorWindow::~EditorWindow()
{
	{
		tke::AttributeTree at("data");

		{
			auto n = new tke::AttributeTreeNode("GameExplorer");
			static bool opened = gameExplorer;
			n->attributes.push_back(new tke::Attribute("opened", &opened));
			at.children.push_back(n);
		}

		for (auto m : monitors)
		{
			auto n = new tke::AttributeTreeNode("Monitor");
			n->attributes.push_back(new tke::Attribute("model_filename", &m->model->filename));
			at.children.push_back(n);
		}

		{
			auto n = new tke::AttributeTreeNode("AttributeWidget");
			static bool opened = attributeWidget;
			n->attributes.push_back(new tke::Attribute("opened", &opened));
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

void EditorWindow::openMonitorWidget(tke::Model *m)
{
	auto monitor = new MonitorWidget(m);
	monitors.push_back(monitor);
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

	for (auto it = monitors.begin(); it != monitors.end(); )
	{
		if (!(*it)->opened)
		{
			delete *it;
			it = monitors.erase(it);
		}
		else
		{
			it++;
		}
	}
	for (auto m : monitors)
		m->show();
	for (auto m : monitors)
	{
		pushCB(m->cb->v, m->renderFinished);
		pushCB(m->transformerTool->cb->v, m->transformerTool->renderFinished);
	}

	ImGui::SetNextWindowPos(ImVec2(0, cy - ImGui::GetItemsLineHeightWithSpacing()));
	ImGui::Begin("status", nullptr, ImVec2(0, 0), 0.3f, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);
	ImGui::Text("FPS:%d", getFPS());
	ImGui::End();

	ui->end();

	pushCB(ui->cb->v, 0);

	endFrame();
}

EditorWindow *mainWindow = nullptr;