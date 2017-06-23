#include "../src/gui.h"

#include "editor.h"
#include "game.h"
#include "monitor.h"
#include "attribute.h"
#include "bone_motion.h"

tke::Image *titleImage = nullptr;

SelectedItem selectedItem;

void SelectedItem::reset()
{
	type = ItemTypeNull;
	ptr = nullptr;
}

void SelectedItem::select(tke::Object *_obj)
{
	if (_obj == ptr) return;

	if (ptr)
		ptr->removeObserver(this);

	type = ItemTypeObject;
	ptr = _obj;

	if (ptr)
		ptr->addObserver(this);

	for (auto o : observers)
		o->listen(this, tke::NotificationTypeChange, _obj);
}

void SelectedItem::listen(void *sender, tke::NotificationType type, void *newData)
{
	ptr = (tke::ObservedObject*)newData;

	for (auto o : observers)
		o->listen(this, tke::NotificationTypeRefresh, ptr);
}

extern SelectedItem selectedItem;

EditorWindow::EditorWindow()
	:Window(800, 600, "TK Engine Editor", true, true, WS_THICKFRAME)
{
	mainWindow = this;

	titleImage = tke::createImage("../misc/title.jpg", true);

	game.load();

	loadUi("ui.xml");
	ImGui::LoadDock("ui_dock.xml");
}

EditorWindow::~EditorWindow()
{
	saveUi("ui.xml");
	ImGui::SaveDock("ui_dock.xml");
}

void EditorWindow::openGameExplorer()
{
	if (!gameExplorer)
		gameExplorer = new GameExplorer;
}

void EditorWindow::openOutputWidget()
{

}

void EditorWindow::openMonitorWidget(const std::string &renderer_filename, tke::Model *m)
{
	auto monitor = new MonitorWidget(renderer_filename, m);
	monitors.push_back(monitor);
}

void EditorWindow::openAttributeWidget()
{
	if (!attributeWidget)
		attributeWidget = new AttributeWidget;
}

void EditorWindow::openBoneMotionWidget()
{
	if (!boneMotionWdiget)
		boneMotionWdiget = new BoneMotionWidget;
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
		if (ImGui::MenuItem("Attribute"))
		{
			openAttributeWidget();
		}
		if (ImGui::MenuItem("Motion"))
		{
			openBoneMotionWidget();
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

	if (attributeWidget)
		attributeWidget->show();

	if (boneMotionWdiget)
		boneMotionWdiget->show();

	for (auto m : monitors)
		m->show();

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

void EditorWindow::saveUi(const std::string &filename)
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
		n->attributes.push_back(new tke::Attribute("renderer_filename", &m->renderer_filename));
		n->attributes.push_back(new tke::Attribute("model_filename", &m->model->filename));
		at.children.push_back(n);
	}

	{
		auto n = new tke::AttributeTreeNode("AttributeWidget");
		static bool opened = attributeWidget;
		n->attributes.push_back(new tke::Attribute("opened", &opened));
		at.children.push_back(n);
	}

	{
		auto n = new tke::AttributeTreeNode("BoneMotionWidget");
		static bool opened = boneMotionWdiget;
		n->attributes.push_back(new tke::Attribute("opened", &opened));
		at.children.push_back(n);
	}

	at.saveXML(filename);
}

void EditorWindow::loadUi(const std::string &filename)
{
	tke::AttributeTree at("data", filename);

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
				auto a_rdrn = c->firstAttribute("renderer_filename");
				auto a_mn = c->firstAttribute("model_filename");
				tke::Model *m = nullptr;
				for (auto _m : game.models)
				{
					if (_m->filename == a_mn->value)
					{
						m = _m->p;
						break;
					}
				}
				openMonitorWidget(a_rdrn->value, m);
			}
			else if (c->name == "AttributeWidget")
			{
				auto a = c->firstAttribute("opened");
				bool opened;
				a->get<bool>(&opened);
				if (opened)
					openAttributeWidget();
			}
			else if (c->name == "BoneMotionWidget")
			{
				auto a = c->firstAttribute("opened");
				bool opened;
				a->get<bool>(&opened);
				if (opened)
					openBoneMotionWidget();
			}
		}
	}
}

EditorWindow *mainWindow = nullptr;