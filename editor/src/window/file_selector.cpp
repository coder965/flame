#include "file_selector.h"
#include "../../../src/utils.h"
#include "../../../src/ui/ui.h"

FileSelector::FileSelector(WindowClass*_pclass, bool _enable_file)
	:Window(_pclass), enable_file(_enable_file)
{
}

void FileSelector::refresh()
{
	dir_list.clear();
	file_list.clear();
	list_index = -1;

	if (!on_refresh())
		return;

	std::experimental::filesystem::directory_iterator end_it;
	for (std::experimental::filesystem::directory_iterator it(path); it != end_it; it++)
	{
		auto str = it->path().filename().string();
		if (std::experimental::filesystem::is_directory(it->status()))
		{
			auto i = std::make_unique<DirItem>();
			i->value = str;
			i->name = ICON_FA_FOLDER_O" " + str;
			dir_list.push_back(std::move(i));
		}
		else if (enable_file)
		{
			auto i = on_new_file_item();

			i->file_size = std::experimental::filesystem::file_size(it->path());

			auto ext = it->path().extension().string();
			const char *prefix;
			if (tke::isTextFile(ext))
			{
				i->file_type = tke::FileTypeText;
				prefix = ICON_FA_FILE_TEXT_O" ";
			}
			else if (tke::isImageFile(ext))
			{
				i->file_type = tke::FileTypeImage;
				prefix = ICON_FA_FILE_IMAGE_O" ";
			}
			else if (tke::isModelFile(ext))
			{
				i->file_type = tke::FileTypeModel;
				prefix = ICON_FA_FILE_O" ";
			}
			else if (tke::isSceneFile(ext))
			{
				i->file_type = tke::FileTypeScene;
				prefix = ICON_FA_FILE_O" ";
			}
			else
				prefix = ICON_FA_FILE_O" ";
			i->value = str;
			i->name = prefix + str;

			on_add_file_item(i);

			file_list.push_back(std::move(std::unique_ptr<FileItem>(i)));
		}
	}
}

void FileSelector::show()
{
	if (need_refresh)
	{
		refresh();
		need_refresh = false;
	}

	auto open = on_window_begin();
	if (open)
	{
		const float itemSpacing = ImGui::GetStyle().ItemSpacing.x;

		ImGui::BeginChild("left", ImVec2(on_left_area_width(), 0));

		on_top_area_begin();

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

		ImGui::BeginChild("list", ImVec2(0, -ImGui::GetItemsLineHeightWithSpacing() - 1), true);
		auto index = 0;
		for (auto &i : dir_list)
		{
			if (ImGui::Selectable(i->name.c_str(), list_index == index, ImGuiSelectableFlags_DontClosePopups | ImGuiSelectableFlags_AllowDoubleClick))
			{
				on_dir_item_selected(i.get());
				list_index = index;
				if (ImGui::IsMouseDoubleClicked(0))
				{
					path /= i->value;
					need_refresh = true;
				}
			}
			index++;
		}
		if (enable_file)
		{
			for (auto &i : file_list)
			{
				if (ImGui::Selectable(i->name.c_str(), index == list_index, ImGuiSelectableFlags_DontClosePopups | ImGuiSelectableFlags_AllowDoubleClick))
				{
					list_index = index;
					on_file_item_selected(i.get(), ImGui::IsMouseDoubleClicked(0));
				}
				index++;
			}
		}
		ImGui::EndChild();

		on_bottom_area_begin();

		ImGui::EndChild();

		on_right_area_begin();

		on_window_end();
	}
}

DirSelectorDialog::DirSelectorDialog()
	:FileSelector(nullptr, false)
{
	filename[0] = 0;
}

bool DirSelectorDialog::on_refresh()
{
	selected_path = "";
	return true;
}

static int driverIndex = 0;
const char *drivers[] = {
	"c:",
	"d:",
	"e:",
	"f:"
};

bool DirSelectorDialog::on_window_begin()
{
	if (first)
	{
		ImGui::OpenPopup("Dir Selector");
		ImGui::SetNextWindowSize(ImVec2(800, 600));
		first = false;
	}
	return ImGui::BeginPopupModal("Dir Selector");
}

void DirSelectorDialog::on_window_end()
{
	ImGui::EndPopup();
}

void DirSelectorDialog::on_top_area_begin()
{
	ImGui::PushItemWidth(100);
	if (ImGui::Combo("##driver", &driverIndex, drivers, TK_ARRAYSIZE(drivers)))
	{
		path = std::string(drivers[driverIndex]) + "\\";
		need_refresh = true;
	}
	ImGui::PopItemWidth();
	ImGui::SameLine();

}

void DirSelectorDialog::on_bottom_area_begin()
{
	static float okButtonWidth = 100;
	static float cancelButtonWidth = 100;

	const float itemSpacing = ImGui::GetStyle().ItemSpacing.x;

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

}

void DirSelectorDialog::on_dir_item_selected(DirItem *i)
{
	selected_path = i->value;
	strcpy(filename, selected_path.c_str());
}

void DirSelectorDialog::open(const std::string &default_dir, const std::function<void(std::string)> &_callback)
{
	auto w = std::make_unique<DirSelectorDialog>();
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
	w->callback = _callback;
	windows.push_back(std::move(w));
}
