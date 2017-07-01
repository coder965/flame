#include "../src/gui.h"
#include "../src/model.h"

#include "game.h"
#include "editor.h"

void Game::load()
{
	{
		tke::AttributeTree at("data", "renderers.xml");
		for (auto c : at.children)
		{
			if (c->name == "renderer")
			{
				auto a = c->firstAttribute("filename");
				auto r = new RendererEditorStruct;
				r->filename = a->value;
				renderers.push_back(r);
			}
		}
	}

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

	if (ImGui::TreeNode("Renderers"))
	{
		for (auto r : game.renderers)
		{
			if (ImGui::Selectable(r->filename.c_str()))
			{

			}
		}
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Models"))
	{
		for (auto m : game.models)
		{
			if (ImGui::Selectable(m->filename.c_str(), false, ImGuiSelectableFlags_AllowDoubleClick))
			{
				if (ImGui::IsMouseDoubleClicked(0))
					mainWindow->openMonitorWidget("../renderer/master.xml", m->p);
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