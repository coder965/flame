#include "../src/core.h"

#include "editor.h"
#include "game.h"
#include "monitor.h"
#include "attribute.h"

tke::Image *titleImage = nullptr;

void SelectedItem::reset()
{
	selected_type = ItemTypeNull;
	selected_ptr = nullptr;
}

void SelectedItem::select(tke::Object *_obj)
{
	if (_obj == selected_ptr) return;

	switch (selected_type)
	{
	case ItemTypeObject:
		((tke::Object*)selected_ptr)->removeObserver(this);
		break;
	case ItemTypeLight:
		//((tke::Light*)selected_ptr)->removeObserver(this);
		break;
	}

	selected_type = ItemTypeObject;
	selected_ptr = _obj;

	_obj->addObserver(this);

	for (auto o : observers)
		o->listen(this, tke::NotificationTypeChange, _obj);
}

void SelectedItem::listen(void *sender, tke::NotificationType type, void *newData)
{
	selected_ptr = newData;
	for (auto o : observers)
		o->listen(this, tke::NotificationTypeRefresh, selected_ptr);
}

extern SelectedItem selectedItem;

EditorWindow::EditorWindow()
	:Window(800, 600, "TK Engine Editor", true, true, WS_THICKFRAME)
{
	titleImage = tke::createImage("../misc/title.jpg", true);

	game.load();
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
			if (!gameExplorer)
				gameExplorer = new GameExplorer;
		}
		if (ImGui::MenuItem("Output"))
		{
		}
		if (ImGui::MenuItem("Attribute"))
		{
			if (!attributeWidget)
				attributeWidget = new AttributeWidget;
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