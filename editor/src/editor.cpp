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
