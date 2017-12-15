#include "file_selector.h"
#include "../../../src/utils.h"
#include "../../../src/ui/ui.h"

FileSelector::FileSelector(WindowClass*_pclass, const std::string &_title, bool _modal, bool _enable_file, int _mode, int _cx, int _cy)
	:Window(_pclass), title(_title), modal(_modal), cx(_cx), cy(_cy), enable_file(_enable_file), mode(_mode)
{
	filename[0] = 0;
}

const char *drivers[5] = {
	"c:",
	"d:",
	"e:",
	"f:"
};

void FileSelector::set_current_path(const std::string &s)
{
	current_path = s;
	driver_index = 0;
	for (int i = 0; i < TK_ARRAYSIZE(drivers); i++)
	{
		if (current_path.root_name() == drivers[i])
		{
			driver_index = i;
			break;
		}
	}
}

void FileSelector::refresh()
{
	dir_list.clear();
	file_list.clear();
	list_index = -1;

	if (!on_refresh())
		return;

	if (current_path == user_define_extra_path)
	{
		on_refresh_user_define_dir();
		return;
	}

	std::experimental::filesystem::directory_iterator end_it;
	for (std::experimental::filesystem::directory_iterator it(current_path); it != end_it; it++)
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

			{
				std::error_code e;
				i->file_size = std::experimental::filesystem::file_size(it->path(), e);
			}

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
			else if (tke::isTerrainFile(ext))
			{
				i->file_type = tke::FileTypeTerrain;
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

void FileSelector::do_show()
{
	if (need_refresh)
	{
		refresh();
		need_refresh = false;
	}

	bool _open;
	if (modal)
	{
		if (first)
		{
			ImGui::OpenPopup(title.c_str());
			ImGui::SetNextWindowSize(ImVec2(cx, cy));
			first = false;
		}
		_open = ImGui::BeginPopupModal(title.c_str(), &opened);
	}
	else
		_open = ImGui::Begin(title.c_str(), &opened);

	if (_open)
	{
		const float itemSpacing = ImGui::GetStyle().ItemSpacing.x;

		ImGui::BeginChild("left", ImVec2(on_left_area_width(), 0));

		on_top_area_show();

		ImGui::PushItemWidth(100);
		auto driver_count = TK_ARRAYSIZE(drivers) - 1;
		if (user_define_extra_path.size() != 0)
		{
			drivers[TK_ARRAYSIZE(drivers) - 1] = user_define_extra_path.c_str();
			driver_count++;
		}
		if (ImGui::Combo("##driver", &driver_index, drivers, driver_count))
		{
			auto d = std::string(drivers[driver_index]);
			if (d != user_define_extra_path)
				d += "\\";
			current_path = d;
			need_refresh = true;
		}
		ImGui::PopItemWidth();
		ImGui::SameLine();
		ImGui::Text(current_path.string().c_str());
		ImGui::SameLine();
		{
			static float buttonWidth = 100.f;
			ImGui::SameLine(ImGui::GetWindowWidth() - buttonWidth - itemSpacing);
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
			if (ImGui::Button(ICON_FA_CHEVRON_UP))
			{
				if (current_path != user_define_extra_path && on_parent_path() && current_path.root_path() != current_path)
				{
					current_path = current_path.parent_path();
					need_refresh = true;
				}
			}
			buttonWidth = ImGui::GetItemRectSize().x;
		}
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Parent Path");
		ImGui::PopStyleColor();
		ImGui::Separator();

		ImGui::BeginChild("list", ImVec2(0, -ImGui::GetFrameHeightWithSpacing() - 1), true);
		auto index = 0;
		for (auto &i : dir_list)
		{
			if (ImGui::Selectable(i->name.c_str(), list_index == index, ImGuiSelectableFlags_DontClosePopups | ImGuiSelectableFlags_AllowDoubleClick))
			{
				strcpy(filename, i->value.c_str());
				i->filename = (current_path / i->value).string();
				on_dir_item_selected(i.get());
				list_index = index;
				if (ImGui::IsMouseDoubleClicked(0))
				{
					current_path /= i->value;
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
					strcpy(filename, i->value.c_str());
					if (current_path != user_define_extra_path)
						i->filename = (current_path / i->value).string();
					else
						i->filename = i->value;
					on_file_item_selected(i.get(), ImGui::IsMouseDoubleClicked(0));
				}
				if (ImGui::BeginDragDropSource())
				{
					if (current_path != user_define_extra_path)
						i->filename = (current_path / i->value).string();
					else
						i->filename = i->value;
					ImGui::SetDragDropPayload("file", i->filename.c_str(), i->filename.size() + 1);
					ImGui::Text(i->filename.c_str());
					ImGui::EndDragDropSource();
				}
				index++;
			}
		}
		ImGui::EndChild();

		on_bottom_area_show();

		ImGui::EndChild();

		on_right_area_show();
	}

	if (modal)
		ImGui::EndPopup();
	else
		ImGui::End();
}

int FileSelector::on_left_area_width() 
{
	return 0; 
}

bool FileSelector::on_refresh() 
{
	return true; 
}

bool FileSelector::on_parent_path() 
{
	return true; 
}

FileSelector::FileItem *FileSelector::on_new_file_item() 
{
	return new FileItem; 
}

void FileSelector::on_add_file_item(FileItem *i) 
{
}

void FileSelector::on_refresh_user_define_dir()
{
}

void FileSelector::on_dir_item_selected(DirItem *i) 
{
}

void FileSelector::on_file_item_selected(FileItem *i, bool doubleClicked) 
{
}

void FileSelector::on_top_area_show() 
{
}

void FileSelector::on_bottom_area_show() 
{
	static float okButtonWidth = 100;
	static float cancelButtonWidth = 100;

	const float itemSpacing = ImGui::GetStyle().ItemSpacing.x;

	ImGui::PushItemWidth(ImGui::GetWindowWidth() - okButtonWidth - cancelButtonWidth - itemSpacing * 4);
	ImGui::InputText("##filename", filename, TK_ARRAYSIZE(filename));
	ImGui::PopItemWidth();
	float pos = okButtonWidth + itemSpacing;
	ImGui::SameLine(ImGui::GetWindowWidth() - pos);
	if (ImGui::Button("  Ok  "))
	{
		auto path = current_path / filename;
		if (mode == 1 || std::experimental::filesystem::exists(path))
		{
			if (callback(path.string()))
			{
				opened = false;
				ImGui::CloseCurrentPopup();
			}
		}
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

void FileSelector::on_right_area_show() 
{
}

DirSelectorDialog::DirSelectorDialog()
	:FileSelector(nullptr, "Dir Selector", true, false, 0, 800, 600)
{
}

void DirSelectorDialog::open(const std::string &default_dir, const std::function<bool(std::string)> &_callback)
{
	auto w = new DirSelectorDialog;
	w->set_current_path(default_dir);
	w->callback = _callback;
}
