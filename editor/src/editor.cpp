#include "../../src/ui/ui.h"
#include "../../src/core.h"

#include "editor.h"
#include "window/dir_selector.h"
#include "window/scene_editor.h"
#include "window/attribute.h"
#include "window/texture_editor.h"

std::experimental::filesystem::path project_path;

int main(int argc, char** argv)
{
	tke::init(true, "../", 800, 600, 1280, 720, "TK Engine Editor", tke::WindowStyleFrame | tke::WindowStyleResize, false);

	ShowWindow(tke::hWnd, SW_SHOWMAXIMIZED);

	initWindow();

	load_resource();

	{
		tke::AttributeTree at("data", "ui.xml");
		if (at.good)
		{
			for (auto &n : at.children)
			{
				if (n->name == "window")
				{
					Window *w = nullptr;
					for (auto c : windowClasses)
					{
						if (w) break;
						auto a = n->firstAttribute("type");
						if (a && a->value == c->getName())
							w = c->load(n.get());
					}
					if (w)
						windows.push_back(std::move(std::unique_ptr<Window>(w)));
				}
				else if (n->name == "object_creation_setting")
					ocs.load(n.get());
				else if (n->name == "terrain_creation_setting")
					tcs.load(n.get());
			}
		}
	}

	tke::onRender = []() {
		tke::beginFrame(true);

		ImGui::BeginMainMenuBar();
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("New Project"))
				;
			if (ImGui::MenuItem("Open Project"))
			{
				DirSelector::open("c:\\", [](std::string path) {
					project_path = path;
					if (resourceExplorer)
					{
						resourceExplorer->path = project_path;
						resourceExplorer->refresh();
					}
				});
			}
			if (ImGui::MenuItem("Save Project"))
				;

			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("View"))
		{
			if (ImGui::MenuItem("Resource Explorer", nullptr, resourceExplorer != nullptr))
			{
				if (!resourceExplorer)
				{
					resourceExplorer = new ResourceExplorer;
					windows.push_back(std::move(std::unique_ptr<Window>(resourceExplorer)));
				}
			}

			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();

		if (textureEditor)
		{
			textureEditor->show();
			if (!textureEditor->opened)
			{
				delete textureEditor;
				textureEditor = nullptr;
			}
		}

		for (auto &w : windows)
			w->show();

		ImGui::SetNextWindowPos(ImVec2(0, tke::window_cy - ImGui::GetItemsLineHeightWithSpacing()));
		ImGui::Begin("status", nullptr, ImVec2(0, 0), 0.3f, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);
		ImGui::Text("FPS:%d", tke::FPS);
		ImGui::End();

		tke::endFrame();

		for (auto it = windows.begin(); it != windows.end(); )
		{
			if (!(*it)->opened)
				it = windows.erase(it);
			else
				it++;
		}
	};

	tke::onDestroy = []() {
		{
			tke::AttributeTree at("data");
			for (auto &w : windows)
			{
				if (!w->pClass)
					continue;
				auto n = new tke::AttributeTreeNode("window");
				n->addAttribute("type", w->pClass->getName());
				w->save(n);
				at.add(n);
			}
			{
				auto n = new tke::AttributeTreeNode("object_creation_setting");
				ocs.save(n);
				at.add(n);
			}
			{
				auto n = new tke::AttributeTreeNode("terrain_creation_setting");
				tcs.save(n);
				at.add(n);
			}
			if (SelectObject)
			{
				auto n = new tke::AttributeTreeNode("select");
				at.add(n);
			}
			at.saveXML("ui.xml");
		}
	};

	tke::run();

	return 0;
}

void ObjectCreationSetting::load(tke::AttributeTreeNode *n)
{
	for (auto &a : n->attributes)
	{
		if (a->name == "modelIndex")
			a->get(&modelIndex);
		else if (a->name == "use_camera_position")
			a->get(&use_camera_position);
		else if (a->name == "use_camera_target_position")
			a->get(&use_camera_target_position);
		else if (a->name == "coord")
			a->get(&coord);
		else if (a->name == "euler")
			a->get(&euler);
		else if (a->name == "scale")
			a->get(&scale);
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
	n->addAttribute("euler", &euler);
	n->addAttribute("scale", &scale);
	n->addAttribute("physxType", &physxType);
}

ObjectCreationSetting ocs;

void TerrainCreationSetting::load(tke::AttributeTreeNode *n)
{
	for (auto &a : n->attributes)
	{
		if (a->name == "coord")
			a->get(&coord);
		else if (a->name == "heightMapIndex")
			a->get(&heightMapIndex);
		else if (a->name == "blendMapIndex")
			a->get(&blendMapIndex);
		else if (a->name == "colorMap0Index")
			a->get(&colorMap0Index);
		else if (a->name == "colorMap1Index")
			a->get(&colorMap1Index);
		else if (a->name == "colorMap2Index")
			a->get(&colorMap2Index);
		else if (a->name == "colorMap3Index")
			a->get(&colorMap3Index);
		else if (a->name == "height")
			a->get(&height);
		else if (a->name == "usePhysx")
			a->get(&usePhysx);
	}
}

void TerrainCreationSetting::save(tke::AttributeTreeNode *n)
{
	n->addAttribute("coord", &coord);
	n->addAttribute("heightMapIndex", &heightMapIndex);
	n->addAttribute("blendMapIndex", &blendMapIndex);
	n->addAttribute("colorMap0Index", &colorMap0Index);
	n->addAttribute("colorMap1Index", &colorMap1Index);
	n->addAttribute("colorMap2Index", &colorMap2Index);
	n->addAttribute("colorMap3Index", &colorMap3Index);
	n->addAttribute("height", &height);
	n->addAttribute("usePhysx", &usePhysx);
}

TerrainCreationSetting tcs;

void WaterCreationSetting::load(tke::AttributeTreeNode *n)
{
	for (auto &a : n->attributes)
	{
		if (a->name == "coord")
			a->get(&coord);
		else if (a->name == "height")
			a->get(&height);
	}
}

void WaterCreationSetting::save(tke::AttributeTreeNode *n)
{
	n->addAttribute("coord", &coord);
	n->addAttribute("height", &height);
}

WaterCreationSetting wcs;