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
					ocs.load(c);
				}
				else if (c->name == "terrain_creation_setting")
				{
					tcs.load(c);
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
			ocs.save(n);
			at.children.push_back(n);
		}
		{
			auto n = new tke::AttributeTreeNode("terrain_creation_setting");
			tcs.save(n);
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
			cbs.push_back(monitorWidget->cb_physx->v);
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

void ObjectCreationSetting::load(tke::AttributeTreeNode *n)
{
	for (auto a : n->attributes)
	{
		if (a->name == "modelIndex")
			a->get(&modelIndex);
		else if (a->name == "use_camera_position")
			a->get(&use_camera_position);
		else if (a->name == "use_camera_target_position")
			a->get(&use_camera_target_position);
		else if (a->name == "coord")
			a->get(&coord);
		else if (a->name == "randCX")
			a->get(&randC[0]);
		else if (a->name == "randCY")
			a->get(&randC[1]);
		else if (a->name == "randCZ")
			a->get(&randC[2]);
		else if (a->name == "coordRandRange")
			a->get(&coordRandRange);
		else if (a->name == "euler")
			a->get(&euler);
		else if (a->name == "randRX")
			a->get(&randR[0]);
		else if (a->name == "randRY")
			a->get(&randR[1]);
		else if (a->name == "randRZ")
			a->get(&randR[2]);
		else if (a->name == "eulerRandRange")
			a->get(&eulerRandRange);
		else if (a->name == "scale")
			a->get(&scale);
		else if (a->name == "randSX")
			a->get(&randS[0]);
		else if (a->name == "randSY")
			a->get(&randS[1]);
		else if (a->name == "randSZ")
			a->get(&randS[2]);
		else if (a->name == "scaleRandRange")
			a->get(&scaleRandRange);
		else if (a->name == "same_scale_rand")
			a->get(&same_scale_rand);
		else if (a->name == "physxType")
			a->get(&physxType);
	}
}

void ObjectCreationSetting::save(tke::AttributeTreeNode *n)
{
	n->addAttribute("modelIndex", &modelIndex);
	n->addAttribute("use_camera_position", &use_camera_position);
	n->addAttribute("use_camera_target_position", &use_camera_target_position);
	n->addAttribute("coord", &coord);
	n->addAttribute("randCX", &randC[0]);
	n->addAttribute("randCY", &randC[1]);
	n->addAttribute("randCZ", &randC[2]);
	n->addAttribute("coordRandRange", &coordRandRange);
	n->addAttribute("euler", &euler);
	n->addAttribute("randRX", &randR[0]);
	n->addAttribute("randRY", &randR[1]);
	n->addAttribute("randRZ", &randR[2]);
	n->addAttribute("eulerRandRange", &eulerRandRange);
	n->addAttribute("scale", &scale);
	n->addAttribute("randSX", &randS[0]);
	n->addAttribute("randSY", &randS[1]);
	n->addAttribute("randSZ", &randS[2]);
	n->addAttribute("scaleRandRange", &scaleRandRange);
	n->addAttribute("same_scale_rand", &same_scale_rand);
	n->addAttribute("physxType", &physxType);
}

ObjectCreationSetting ocs;

void TerrainCreationSetting::load(tke::AttributeTreeNode *n)
{
	for (auto a : n->attributes)
	{
		if (a->name == "heightMapIndex")
			a->get(&heightMapIndex);
		else if (a->name == "colorMapIndex")
			a->get(&colorMapIndex);
		else if (a->name == "height")
			a->get(&height);
		else if (a->name == "usePhysx")
			a->get(&usePhysx);
	}
}

void TerrainCreationSetting::save(tke::AttributeTreeNode *n)
{
	n->addAttribute("heightMapIndex", &heightMapIndex);
	n->addAttribute("colorMapIndex", &colorMapIndex);
	n->addAttribute("height", &height);
	n->addAttribute("usePhysx", &usePhysx);
}

TerrainCreationSetting tcs;