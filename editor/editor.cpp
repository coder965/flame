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
	titleImage = tke::createImage("../misc/title.jpg", true);

	game.load();

	on_view_gameExplorer();
	on_view_attributeWidget();
	on_view_boneMotionWidget();
}

void EditorWindow::on_view_gameExplorer()
{
	if (!gameExplorer)
		gameExplorer = new GameExplorer;
}

void EditorWindow::on_view_output()
{

}

void EditorWindow::on_view_attributeWidget()
{
	if (!attributeWidget)
		attributeWidget = new AttributeWidget;
}

void EditorWindow::on_view_boneMotionWidget()
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
			on_view_gameExplorer();
		}
		if (ImGui::MenuItem("Output"))
		{
		}
		if (ImGui::MenuItem("Attribute"))
		{
			on_view_attributeWidget();
		}
		if (ImGui::MenuItem("Motion"))
		{
			on_view_boneMotionWidget();
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

EditorWindow *mainWindow = nullptr;