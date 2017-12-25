#include "../../src/ui/ui.h"
#include "../../src/core.h"

#include "window/file_selector.h"
#include "window/resource_explorer.h"
#include "window/scene_editor.h"
#include "window/image_editor.h"

struct NewImageDialog : FileSelector
{
	bool first = true;
	int cx = 512;
	int cy = 512;

	NewImageDialog()
		:FileSelector("New Image", true, true, 1, 800, 600)
	{
		callback = [this](std::string s) {
			if (std::experimental::filesystem::exists(s))
				return false;
			tke::newImageFile(s, cx, cy, 32);
			return true;
		};
	}

	virtual int on_left_area_width() override
	{
		return 300;
	}

	virtual void on_right_area_show() override
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
	tke::init(true, "../", 1920, 1080, 1280, 720, "TK Engine Editor", tke::WindowStyleFrame | tke::WindowStyleResize, false);

	ShowWindow(tke::hWnd, SW_SHOWMAXIMIZED);

	{
		tke::XMLDoc at("data", "ui.xml");
		if (at.good)
		{
			for (auto &n : at.children)
			{
				if (n->name == "resource_explorer")
					resourceExplorer = new ResourceExplorer;
			}
		}
	}

	tke::onRender = []() {
		tke::beginFrame(true);

		if (ImGui::last_frame_main_menu_alive || tke::mouseY <= ImGui::GetFrameHeight())
		{
			ImGui::BeginMainMenuBar();
			if (ImGui::BeginMenu_keepalive("File"))
			{
				if (ImGui::BeginMenu("New"))
				{
					if (ImGui::MenuItem("Scene"))
						;
					if (ImGui::MenuItem("Image"))
						new NewImageDialog;
					if (ImGui::MenuItem("Terrain"))
						;

					ImGui::EndMenu();
				}

				if (scene_editor)
					scene_editor->on_file_menu();

				ImGui::EndMenu();
			}
			if (scene_editor)
				scene_editor->on_menu_bar();
			if (ImGui::BeginMenu_keepalive("View"))
			{
				static bool fullscreen = false;
				if (ImGui::MenuItem("Fullscreen", "", &fullscreen))
				{
					if (fullscreen)
					{
						tke::window_style |= tke::WindowStyleFullscreen;
						tke::window_style &= (~tke::WindowStyleFrame);
					}
					else
					{
						tke::window_style |= tke::WindowStyleFrame;
						tke::window_style &= (~tke::WindowStyleFullscreen);
						tke::window_cx = tke::resCx;
						tke::window_cy = tke::resCy;
					}
					auto wndProp = tke::getWin32WndProp();
					SetWindowLong(tke::hWnd, GWL_STYLE, wndProp.second);
					SetWindowPos(tke::hWnd, HWND_TOP, 0, 0, wndProp.first.x, wndProp.first.y, SWP_NOZORDER);
				}
				ImGui::Separator();
				if (ImGui::MenuItem("Resource Explorer"))
				{
					if (!resourceExplorer)
						resourceExplorer = new ResourceExplorer;
					resourceExplorer->_need_focus = true;
				}
				if (scene_editor)
					scene_editor->on_view_menu();

				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}

		if (scene_editor)
		{
			ImGui::SetNextWindowPos(ImVec2(0, 0));
			ImGui::SetNextWindowSize(ImVec2(tke::window_cx, tke::window_cy));
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
			if (ImGui::Begin("Main", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | 
				ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoFocusOnAppearing | 
				ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoSavedSettings))
				scene_editor->do_show();
			ImGui::End();
			ImGui::PopStyleVar(2);
		}

		{
			std::vector<IWindow*> _w;
			for (auto &w : windows)
				_w.push_back(w.get());
			for (auto &w : _w)
				w->show();
		}

		ImGui::SetNextWindowPos(ImVec2(0, tke::window_cy - ImGui::GetFrameHeightWithSpacing()));
		ImGui::Begin("status", nullptr, ImVec2(0, 0), 0.3f, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | 
			ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);
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
		tke::XMLDoc at("data");
		if (resourceExplorer)
			at.newNode("resource_explorer");
		if (SelectObject)
			at.newNode("select");
		at.save("ui.xml");
	};

	tke::run();

	return 0;
}
