#include <filesystem>

#include "../src/gui.h"
#include "../src/entity.h"

#include "game.h"
#include "editor.h"

void Game::load()
{
	tke::AttributeTree at("data", "data.xml");
	for (auto &c : at.children)
	{
		if (c->name == "texture")
		{
			auto a = c->firstAttribute("filename");
			auto i = tke::createImage(a->value, false, true);
			if (i)
			{
				tke::addTexture(i);
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
			auto s = new tke::Scene;
			s->load(a->value);
			s->camera.setMode(tke::CameraMode::targeting);
			s->camera.setCoord(0.f, 5.f, 0.f);
			tke::addScene(s);
		}
		else if (c->name == "dir")
		{
			auto a = c->firstAttribute("path");
			tke::iterateDirectory(a->value, [](const std::experimental::filesystem::path &name, bool is_dir) {
				if (!is_dir)
				{
					auto ext = name.extension().string();
					if (ext == ".bmp" || ext == ".jpg" || ext == ".jpeg" || ext == ".png" || ext == ".tga")
					{
						auto i = tke::createImage(name.string(), false, true);
						if (i)
						{
							tke::addTexture(i);
							tke::addGuiImage(i);
						}
					}
					else if (ext == ".obj" || ext == ".pmd" || ext == ".dae" || ext == ".tkm")
					{
						tke::createModel(name.string());
					}
					else if (ext == ".vmd" || ext == ".t3a")
					{
						tke::createAnimation(name.string());
					}
				}
			});

			std::experimental::filesystem::path path(a->value);
			std::experimental::filesystem::directory_iterator end_it;
			for (std::experimental::filesystem::directory_iterator it(path); it != end_it; it++)
			{

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

	if (ImGui::TreeNode("Textures"))
	{
		for (int i = 0; i < tke::textures.size(); i++)
		{
			auto t = tke::textures[i].get();
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
			auto a = tke::animations[i].get();
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
			auto m = tke::models[i].get();
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
		for (int i = 0; i < tke::scenes.size(); i++)
		{
			auto s = tke::scenes[i].get();
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