#include "dir_selector.h"
#include "../../../src/utils.h"
#include "../../../src/ui/ui.h"

DirSelector::DirSelector()
	:Window(nullptr)
{
}

void DirSelector::refresh()
{
	dir_list.clear();
	list_index = -1;
	selected_path = "";

	std::experimental::filesystem::directory_iterator end_it;
	for (std::experimental::filesystem::directory_iterator it(path); it != end_it; it++)
	{
		if (std::experimental::filesystem::is_directory(it->status()))
		{
			auto str = it->path().filename().string();
			auto i = std::make_unique<DirSelectorListItem>();
			i->value = str;
			i->name = ICON_FA_FOLDER_O" " + str;
			dir_list.push_back(std::move(i));
		}
	}
}

static int driverIndex = 0;
const char *drivers[] = {
	"c:",
	"d:",
	"e:",
	"f:"
};

void DirSelector::open(const std::string &default_dir, const std::function<void(std::string)> &_callback)
{
	auto w = std::make_unique<DirSelector>();
	w->path = default_dir;
	driverIndex = 0;
	for (int i = 0; i < TK_ARRAYSIZE(drivers); i++)
	{
		if (w->path.root_name() == drivers[i])
		{
			driverIndex = i;
			break;
		}
	}
	w->refresh();
	w->callback = _callback;
	windows.push_back(std::move(w));
}

void DirSelector::show()
{
	if (first)
	{
		ImGui::OpenPopup("Dir Selector");
		ImGui::SetNextWindowSize(ImVec2(800, 600));
		first = false;
	}
	if (ImGui::BeginPopupModal("Dir Selector"))
	{
		const float itemSpacing = ImGui::GetStyle().ItemSpacing.x;
		bool need_refresh = false;

		ImGui::PushItemWidth(100);
		if (ImGui::Combo("##driver", &driverIndex, drivers, TK_ARRAYSIZE(drivers)))
		{
			path = std::string(drivers[driverIndex]) + "\\";
			need_refresh = true;
		}
		ImGui::PopItemWidth();
		ImGui::SameLine();
		ImGui::Text(path.string().c_str());
		ImGui::SameLine();
		{
			static float buttonWidth = 100.f;
			ImGui::SameLine(ImGui::GetWindowWidth() - buttonWidth - itemSpacing);
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
			if (ImGui::Button(ICON_FA_CHEVRON_UP))
			{
				if (path.root_path() != path)
				{
					path = path.parent_path();
					need_refresh = true;
				}
			}
			buttonWidth = ImGui::GetItemRectSize().x;
		}
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Parent Path");
		ImGui::PopStyleColor();
		ImGui::Separator();

		static char filename[260];

		ImGui::BeginChild("list", ImVec2(0, -ImGui::GetItemsLineHeightWithSpacing()), true);
		for (int i = 0; i < dir_list.size(); i++)
		{
			if (ImGui::Selectable(dir_list[i]->name.c_str(), list_index == i, ImGuiSelectableFlags_DontClosePopups| ImGuiSelectableFlags_AllowDoubleClick))
			{
				selected_path = dir_list[i]->value;
				strcpy(filename, selected_path.c_str());
				list_index = i;
				if (ImGui::IsMouseDoubleClicked(0))
				{
					path /= dir_list[i]->value;
					need_refresh = true;
				}
			}
		}
		ImGui::EndChild();
		if (need_refresh)
			refresh();

		static float okButtonWidth = 100;
		static float cancelButtonWidth = 100;

		ImGui::PushItemWidth(ImGui::GetWindowWidth() - okButtonWidth - cancelButtonWidth - itemSpacing * 4);
		if (ImGui::InputText("##filename", filename, TK_ARRAYSIZE(filename)))
			selected_path = filename;
		ImGui::PopItemWidth();
		float pos = okButtonWidth + itemSpacing;
		ImGui::SameLine(ImGui::GetWindowWidth() - pos);
		if (ImGui::Button("  Ok  "))
		{
			callback((path / selected_path).string());
			opened = false;
			ImGui::CloseCurrentPopup();
		}
		okButtonWidth = ImGui::GetItemRectSize().x;

		pos += cancelButtonWidth + itemSpacing;
		ImGui::SameLine(ImGui::GetWindowWidth() - pos);
		if (ImGui::Button("Cancel"))
		{
			opened = false;
			ImGui::CloseCurrentPopup();
		}
		cancelButtonWidth = ImGui::GetItemRectSize().x;

		ImGui::EndPopup();
	}
}
