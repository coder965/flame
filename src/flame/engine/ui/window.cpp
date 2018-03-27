#include <flame/engine/ui/window.h>

namespace flame
{
	namespace ui
	{
		static std::list<std::unique_ptr<Window>> windows;

		Window::Window(const std::string &_title, unsigned int flags) :
			first(true),
			first_cx(0),
			first_cy(0),
			_need_focus(false),
			_tag(WindowTagNull),
			enable_menu(flags & WindowHasMenu),
			enable_saved_settings(!(flags & WindowNoSavedSettings)),
			modal(flags & WindowModal),
			opened(true),
			layout(nullptr),
			idx(-1)
		{
			title = _title;
			enable_dock = !(flags & WindowBanDock);
			windows.emplace_back(this);
		}

		void Window::show()
		{
			if (!layout)
			{
				if (_need_focus)
				{
					ImGui::SetNextWindowFocus();
					_need_focus = false;
				}
				if (first)
				{
					if (first_cx != 0 && first_cy != 0)
						ImGui::SetNextWindowSize(ImVec2(first_cx, first_cy));
				}

				if (!modal)
				{
					if (ImGui::Begin(title.c_str(), &opened, (enable_menu ? ImGuiWindowFlags_MenuBar : 0) |
						(!enable_saved_settings ? ImGuiWindowFlags_NoSavedSettings : 0)))
					{
						if (ImGui::IsItemActive() && ImGui::IsItemHovered())
							set_dragging_window(this);

						on_show();
					}
					if (_tag == WindowTagUndock)
					{
						auto g = ImGui::GetCurrentContext();
						auto w = ImGui::GetCurrentWindow();
						ImGui::SetActiveID(w->MoveId, w);
						g->MovingWindow = w;
					}
					ImGui::End();
				}
				else
				{
					if (first)
						ImGui::OpenPopup(title.c_str());
					auto _open = ImGui::BeginPopupModal(title.c_str(), &opened);
					if (_open)
					{
						on_show();
						ImGui::EndPopup();
					}
				}

				first = false;
			}
			else
				on_show();
		}

		const std::list<std::unique_ptr<Window>> &get_windows()
		{
			return windows;
		}

		void show_windows()
		{
			for (auto &w : windows)
			{
				if (w->_tag == WindowTagClose)
				{
					undock(w.get());
					w->opened = false;
				}
			}

			for (auto it = windows.begin(); it != windows.end(); )
			{
				if (!(*it)->opened)
					it = windows.erase(it);
				else
					it++;
			}

			for (auto &w : windows)
			{
				if (w->_tag == WindowTagUndock)
					undock(w.get());
				if (!w->layout)
					w->show();
				w->_tag = WindowTagNull;
			}
		}
	}
}
