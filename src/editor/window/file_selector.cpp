#include "../../global.h"
#include "../../system.h"
#include "../../ui/ui.h"
#include "file_selector.h"

FileSelector::FileSelector(const std::string &_title, FileSelectorIo io, unsigned int window_flags, unsigned int 
	flags) :
	Window(_title, window_flags),
	io_mode(io),
	enable_file(!(flags & FileSelectorNoFiles)),
	enable_right_area(!(flags & FileSelectorNoRightArea)), 
	tree_mode(flags & FileSelectorTreeMode),
	splitter(true)
{
	splitter.size[0] = 300;
	filename[0] = 0;
	set_current_path(tke::get_exe_path());
}

const char *drivers[] = {
	"c:",
	"d:",
	"e:",
	"f:",
	""
};

void FileSelector::set_current_path(const std::string &s, bool need_choose_driver)
{
	curr_dir.filename = s;
	need_refresh = true;
	if (!tree_mode)
	{

	}
	else
	{
		std::fs::path path(s);
		auto str = path.filename().string();
		curr_dir.value = str;
		curr_dir.name = ICON_FA_FOLDER_O" " + str;
	}
	if (need_choose_driver)
	{
		driver_index = 0;
		std::fs::path path(s);
		auto root_name = path.root_name().string();
		std::transform(root_name.begin(), root_name.end(), root_name.begin(), tolower);
		for (int i = 0; i < TK_ARRAYSIZE(drivers); i++)
		{
			if (root_name == drivers[i])
			{
				driver_index = i;
				break;
			}
		}
	}
}

void FileSelector::refresh()
{
	curr_dir.dir_list.clear();
	curr_dir.file_list.clear();
	select_index = -1;
	select_dir = nullptr;

	if (!on_refresh())
		return;

	if (curr_dir.filename == user_define_extra_path)
	{
		on_refresh_user_define_dir();
		return;
	}

	std::function<void(DirItem *, const std::fs::path &)> fIterDir;
	fIterDir = [this, &fIterDir](DirItem *dst, const std::fs::path &src) {
		std::fs::directory_iterator end_it;
		for (std::fs::directory_iterator it(src); it != end_it; it++)
		{
			auto str = it->path().filename().string();
			if (std::fs::is_directory(it->status()))
			{
				auto i = new DirItem;
				i->value = str;
				i->name = ICON_FA_FOLDER_O" " + str;
				i->filename = it->path().string();
				if (tree_mode)
					fIterDir(i, it->path());
				dst->dir_list.emplace_back(i);
			}
			else if (enable_file)
			{
				auto i = on_new_file_item();

				auto ext = it->path().extension().string();
				const char *prefix;
				if (tke::is_text_file(ext))
				{
					i->file_type = tke::FileTypeText;
					prefix = ICON_FA_FILE_TEXT_O" ";
				}
				else if (tke::is_image_file(ext))
				{
					i->file_type = tke::FileTypeImage;
					prefix = ICON_FA_FILE_IMAGE_O" ";
				}
				else if (tke::is_model_file(ext))
				{
					i->file_type = tke::FileTypeModel;
					prefix = ICON_FA_FILE_O" ";
				}
				else if (tke::is_terrain_file(ext))
				{
					i->file_type = tke::FileTypeTerrain;
					prefix = ICON_FA_FILE_O" ";
				}
				else if (tke::is_scene_file(ext))
				{
					i->file_type = tke::FileTypeScene;
					prefix = ICON_FA_FILE_O" ";
				}
				else
					prefix = ICON_FA_FILE_O" ";
				i->value = str;
				i->name = prefix + str;
				i->filename = it->path().string();

				on_add_file_item(i);

				dst->file_list.emplace_back(i);
			}
		}
	};

	fIterDir(&curr_dir, curr_dir.filename);
}

void FileSelector::on_show()
{
	if (need_refresh)
	{
		refresh();
		need_refresh = false;
	}

	const float itemSpacing = ImGui::GetStyle().ItemSpacing.x;

	if (enable_right_area)
	{
		splitter.set_size_greedily();
		splitter.do_split();
		ImGui::BeginChild("left", ImVec2(splitter.size[0], 0), true);
	}

	if (!tree_mode)
	{
		on_top_area_show();

		auto driver_count = TK_ARRAYSIZE(drivers) - 1;
		if (user_define_extra_path.size() != 0)
		{
			drivers[TK_ARRAYSIZE(drivers) - 1] = user_define_extra_path.c_str();
			driver_count++;
		}
		if (ImGui::IconButton(ICON_FA_CHEVRON_RIGHT))
			ImGui::OpenPopup("DriverPopup");
		if (ImGui::BeginPopup("DriverPopup"))
		{
			for (int i = 0; i < driver_count; i++)
			{
				if (ImGui::Selectable(drivers[i], driver_index == i))
				{
					driver_index = i;
					auto d = std::string(drivers[driver_index]);
					if (d != user_define_extra_path)
						d += "\\";
					set_current_path(d);
				}
			}
			ImGui::EndPopup();
		}
		ImGui::SameLine();
		{
			auto offset = 5.f;
			auto pos = ImGui::GetCursorPosY();
			ImGui::SetCursorPosY(pos - offset);
			ImGui::Text(curr_dir.filename.c_str());
			pos = ImGui::GetCursorPosY();
			ImGui::SetCursorPosY(pos + offset);
		}
		ImGui::Separator();

		ImGui::BeginChild("list", ImVec2(0, -ImGui::GetFrameHeightWithSpacing() - 1), true);
		auto index = 0;
		if (ImGui::Selectable(ICON_FA_FOLDER_O" ..", select_index == index, ImGuiSelectableFlags_DontClosePopups | ImGuiSelectableFlags_AllowDoubleClick))
		{
			select_index = index;
			if (ImGui::IsMouseDoubleClicked(0))
			{
				if (curr_dir.filename != user_define_extra_path && on_parent_path())
				{
					std::fs::path path(curr_dir.filename);
					if (path.root_path() != path)
						set_current_path(path.parent_path().string());
				}
			}
		}
		index++;
		for (auto &i : curr_dir.dir_list)
		{
			if (ImGui::Selectable(i->name.c_str(), select_index == index, ImGuiSelectableFlags_DontClosePopups | ImGuiSelectableFlags_AllowDoubleClick))
			{
				strcpy(filename, i->value.c_str());
				on_dir_item_selected(i.get());
				select_index = index;
				if (ImGui::IsMouseDoubleClicked(0))
					set_current_path((std::fs::path(curr_dir.filename) / i->value).string());
			}
			index++;
		}
		if (enable_file)
		{
			for (auto &i : curr_dir.file_list)
			{
				if (ImGui::Selectable(i->name.c_str(), index == select_index, ImGuiSelectableFlags_DontClosePopups | ImGuiSelectableFlags_AllowDoubleClick))
				{
					select_index = index;
					strcpy(filename, i->value.c_str());
					on_file_item_selected(i.get(), ImGui::IsMouseDoubleClicked(0));
				}
				if (ImGui::BeginDragDropSource())
				{
					ImGui::SetDragDropPayload("file", i->filename.c_str(), i->filename.size() + 1);
					ImGui::TextUnformatted(i->filename.c_str());
					ImGui::EndDragDropSource();
				}
				index++;
			}
		}
		ImGui::EndChild();

		on_bottom_area_show();
	}
	else
	{
		std::function<void(DirItem *)> fShowDir;
		fShowDir = [this, &fShowDir](DirItem *src) {
			auto node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
			node_flags |= select_dir == src ? ImGuiTreeNodeFlags_Selected : 0;
			if (src->dir_list.size() > 0)
			{
				auto node_open = ImGui::TreeNodeEx(src, node_flags, src->name.c_str());
				if (ImGui::IsItemClicked())
					select_dir = src;
				if (node_open)
				{
					for (auto &d : src->dir_list)
						fShowDir(d.get());
					ImGui::TreePop();
				}
			}
			else
			{
				node_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
				ImGui::TreeNodeEx(src, node_flags, src->name.c_str());
				if (ImGui::IsItemClicked())
					select_dir = src;
			}
		};
		fShowDir(&curr_dir);
	}

	if (enable_right_area)
	{
		ImGui::EndChild();
		ImGui::SameLine();
		ImGui::BeginChild("right", ImVec2(0, 0), true);
		on_right_area_show();
		ImGui::EndChild();
	}
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

	ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1);
	ImGui::PushItemWidth(ImGui::GetWindowWidth() - okButtonWidth - cancelButtonWidth - itemSpacing * 4);
	ImGui::InputText("##filename", filename, TK_ARRAYSIZE(filename));
	ImGui::PopItemWidth();
	ImGui::PopStyleVar();

	float pos = okButtonWidth + itemSpacing;
	ImGui::SameLine(ImGui::GetWindowWidth() - pos);
	if (ImGui::Button("  Ok  "))
	{
		auto path = std::fs::path(curr_dir.filename) / filename;
		if (io_mode == FileSelectorSave || std::fs::exists(path))
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

DirSelectorDialog::DirSelectorDialog() :
	FileSelector("Dir Selector", FileSelectorOpen, tke::ui::WindowModal | tke::ui::WindowNoSavedSettings, FileSelectorNoFiles | FileSelectorNoRightArea)
{
	first_cx = 800;
	first_cy = 600;
}

void DirSelectorDialog::open(const std::string &default_dir, const std::function<bool(std::string)> &_callback)
{
	auto w = new DirSelectorDialog;
	w->set_current_path(default_dir);
	w->callback = _callback;
}
