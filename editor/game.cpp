#include "../src/gui.h"
#include "../src/entity.h"

#include "game.h"
#include "editor.h"

void Game::load()
{
	tke::AttributeTree at("data", "data.xml");
	for (auto c : at.children)
	{
		if (c->name == "texture")
		{
			auto a = c->firstAttribute("filename");
			auto i = tke::createImage(a->value, false, true);
			if (i)
			{
				tke::textures.push_back(i);
				tke::addGuiImage(i);
			}
		}
		else if (c->name == "model")
		{
			auto a = c->firstAttribute("filename");
			tke::createModel(a->value);
		}
		else if (c->name == "animation")
		{
			auto a = c->firstAttribute("filename");
			tke::createAnimation(a->value);
		}
		else if (c->name == "scene")
		{
			auto a = c->firstAttribute("filename");
			auto scene = new tke::Scene;
			scene->load(a->value);
			scene->camera.setMode(tke::CameraMode::targeting);
			scene->camera.setCoord(0.f, 5.f, 0.f);
			scenes.push_back(scene);
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

	if (ImGui::TreeNode("Textures"))
	{
		for (int i = 0; i < tke::textures.size(); i++)
		{
			auto t = tke::textures[i];
			if (ImGui::Selectable(t->filename.c_str(), lastItemType == lastItemTypeTexture && itemIndex == i))
			{
				lastItemType = lastItemTypeTexture;
				itemIndex = i;
			}
		}
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Animations"))
	{
		for (int i = 0; i < tke::animations.size(); i++)
		{
			auto a = tke::animations[i];
			if (ImGui::Selectable(a->filename.c_str(), lastItemType == lastItemTypeAnimation && itemIndex == i, ImGuiSelectableFlags_AllowDoubleClick))
			{
				lastItemType = lastItemTypeAnimation;
				itemIndex = i;
				if (ImGui::IsMouseDoubleClicked(0))
					;
			}
		}
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Models"))
	{
		for (int i = 0; i < tke::models.size(); i++)
		{
			auto m = tke::models[i];
			if (ImGui::Selectable(m->filename.c_str(), lastItemType == lastItemTypeModel && itemIndex == i, ImGuiSelectableFlags_AllowDoubleClick))
			{
				lastItemType = lastItemTypeModel;
				itemIndex = i;
				if (ImGui::IsMouseDoubleClicked(0))
					mainWindow->openModelMonitorWidget(m);
			}
		}
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Scenes"))
	{
		for (int i = 0; i < game.scenes.size(); i++)
		{
			auto s = game.scenes[i];
			if (ImGui::Selectable(s->name.c_str(), lastItemType == lastItemTypeScene && itemIndex == i, ImGuiSelectableFlags_AllowDoubleClick))
			{
				lastItemType = lastItemTypeScene;
				itemIndex = i;
				if (ImGui::IsMouseDoubleClicked(0))
					mainWindow->openSceneMonitorWidget(s);
			}
		}
		ImGui::TreePop();
	}

	ImGui::EndDock();
}

GameExplorer *gameExplorer = nullptr;