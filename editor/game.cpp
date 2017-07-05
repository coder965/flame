#include "../src/gui.h"
#include "../src/model.h"

#include "game.h"
#include "editor.h"

void Game::load()
{
	{
		tke::AttributeTree at("data", "models.xml");
		for (auto c : at.children)
		{
			if (c->name == "model")
			{
				auto a = c->firstAttribute("filename");
				auto m = new ModelEditorStruct;
				m->filename = a->value;
				m->p = tke::createModel(a->value);
				models.push_back(m);
			}
		}
	}
}

void Game::save()
{

}

Game game;

void GameExplorer::show()
{
	ImGui::BeginDock("Game Explorer", &opened);
	if (ImGui::IsWindowFocused())
		lastWindowType = LastWindowTypeGameExplorer;

	if (ImGui::TreeNode("Models"))
	{
		for (int i = 0; i < game.models.size(); i++)
		{
			auto m = game.models[i];
			if (ImGui::Selectable(m->filename.c_str(), false, ImGuiSelectableFlags_AllowDoubleClick))
			{
				lastItemType = lastItemTypeModel;
				itemIndex = i;
				if (ImGui::IsMouseDoubleClicked(0))
					mainWindow->openMonitorWidget(m->p);
			}
		}
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Scenes"))
	{
		ImGui::TreePop();
	}

	ImGui::EndDock();
}

GameExplorer *gameExplorer = nullptr;