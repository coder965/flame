#include "../../src/ui/ui.h"
#include "../../src/core.h"

#include "editor.h"
#include "window/file_selector.h"
#include "window/resource_explorer.h"
#include "window/scene_editor.h"
#include "window/image_editor.h"

std::experimental::filesystem::path project_path = "d:\\";

struct NewImageDialog : FileSelector
{
	bool first = true;
	int cx = 512;
	int cy = 512;

	NewImageDialog()
		:FileSelector(nullptr, true, 1)
	{
		callback = [this](std::string s) {
			if (std::experimental::filesystem::exists(s))
				return false;
			tke::newImageFile(s, cx, cy, 32);
			return true;
		};
		set_current_path(project_path.string());
	}

	virtual bool on_window_begin() override 
	{
		if (first)
		{
			ImGui::OpenPopup("New Image");
			ImGui::SetNextWindowSize(ImVec2(800, 600));
			first = false;
		}
		return ImGui::BeginPopupModal("New Image");
	}

	virtual void on_window_end() override
	{
		ImGui::EndPopup();
	}

	virtual int on_left_area_width() override
	{
		return 300;
	}

	virtual void on_right_area_begin() override
	{
		ImGui::SameLine();
		ImGui::BeginGroup();
		ImGui::PushItemWidth(200);
		ImGui::DragInt("cx", &cx);
		ImGui::DragInt("cy", &cy);
		const char *typeNames[] = {
			"color R8G8B8A8"
		};
		static int type = 0;
		ImGui::Combo("type", &type, typeNames, TK_ARRAYSIZE(typeNames));
		ImGui::PopItemWidth();
		ImGui::EndGroup();
	}
};

int main(int argc, char** argv)
{
	tke::init(true, "../", 800, 600, 1280, 720, "TK Engine Editor", tke::WindowStyleFrame | tke::WindowStyleResize, false);

	ShowWindow(tke::hWnd, SW_SHOWMAXIMIZED);

	initWindow();

	{
		tke::AttributeTree at("data", "ui.xml");
		if (at.good)
		{
			for (auto &n : at.children)
			{
				if (n->name == "window")
				{
					for (auto c : windowClasses)
					{
						auto a = n->firstAttribute("type");
						if (a && a->value == c->getName())
							if (c->load(n.get()))
								break;
					}
				}
			}
		}
	}

	tke::onRender = []() {
		tke::beginFrame(true);

		ImGui::BeginMainMenuBar();
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::BeginMenu("New"))
			{
				if (ImGui::MenuItem("Project"))
					;
				if (ImGui::MenuItem("Image"))
					new NewImageDialog;
				if (ImGui::MenuItem("Terrain"))
					;

				ImGui::EndMenu();
			}
			if (ImGui::MenuItem("Open Project"))
			{
				DirSelectorDialog::open("c:\\", [](std::string path) {
					project_path = path;
					if (resourceExplorer)
					{
						resourceExplorer->current_path = project_path;
						resourceExplorer->refresh();
					}
					return true;
				});
			}
			if (ImGui::MenuItem("Save Project"))
				;
			if (ImGui::MenuItem("Close Project"))
				;

			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("View"))
		{
			if (ImGui::MenuItem("Resource Explorer", nullptr, resourceExplorer != nullptr))
			{
				if (!resourceExplorer)
					resourceExplorer = new ResourceExplorer;
			}

			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();

		{
			std::vector<Window*> _w;
			for (auto &w : windows)
				_w.push_back(w.get());
			for (auto &w : _w)
				w->show();
		}

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
