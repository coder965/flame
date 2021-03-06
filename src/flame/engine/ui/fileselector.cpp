#include <flame/global.h>
#include <flame/string.h>
#include <flame/engine/graphics/texture.h>
#include <flame/engine/ui/fileselector.h>

namespace flame
{
	namespace ui
	{
		FileSelector::FileItem::FileItem(FileSelector *_parent) :
			parent(_parent),
			file_type(FileTypeUnknown)
		{
		}

		FileSelector::FileItem::~FileItem()
		{
			if (preview_image && preview_image != parent->empty_image)
				decrease_texture_ref(preview_image.get());
		}

		FileSelector::FileSelector(const std::string &_title, FileSelectorIo io, const std::string &_default_dir,
			unsigned int window_flags, unsigned int flags) :
			Window(_title, window_flags),
			io_mode(io),
			enable_file(!(flags & FileSelectorNoFiles)),
			enable_right_area(!(flags & FileSelectorNoRightArea)),
			tree_mode(flags & FileSelectorTreeMode),
			splitter(true)
		{
			file_watcher = nullptr;

			splitter.size[0] = 300;
			filename[0] = 0;
			if (_default_dir != "")
			{
				default_dir = _default_dir;
				set_current_path(default_dir);
			}
			else
				set_current_path(get_app_path());

			if (tree_mode)
			{
				folder_image = get_texture("folder.png");
				file_image = get_texture("file.png");
				empty_image = get_texture("empty.png");
				
				assert(folder_image);
				assert(file_image);
				assert(empty_image);

				flame::ui::increase_texture_ref(folder_image.get());
				flame::ui::increase_texture_ref(file_image.get());
				flame::ui::increase_texture_ref(empty_image.get());
			}
		}

		FileSelector::~FileSelector()
		{
			if (tree_mode)
			{
				flame::ui::decrease_texture_ref(folder_image.get());
				flame::ui::decrease_texture_ref(file_image.get());
				flame::ui::decrease_texture_ref(empty_image.get());
			}
		}

		const char *drivers[] = {
			"c:",
			"d:",
			"e:",
			"f:"
		};

		void FileSelector::set_current_path(const std::string &s)
		{
			if (s == curr_dir.filename)
				return;

			curr_dir.filename = s;
			need_refresh = true;
			if (!tree_mode)
			{
				curr_dir_hierarchy.clear();
				if (default_dir == "")
				{
					std::filesystem::path path(curr_dir.filename);
					auto root_path = path.root_path();
					while (path != root_path)
					{
						curr_dir_hierarchy.insert(curr_dir_hierarchy.begin(), path.filename().string());
						path = path.parent_path();
					}
					curr_dir_hierarchy.insert(curr_dir_hierarchy.begin(), string_cut(root_path.string(), -1));
					curr_dir_hierarchy.insert(curr_dir_hierarchy.begin(), "");
				}
				else
				{
					std::filesystem::path path(curr_dir.filename);
					while (path != default_dir)
					{
						curr_dir_hierarchy.insert(curr_dir_hierarchy.begin(), path.filename().string());
						path = path.parent_path();
					}
					curr_dir_hierarchy.insert(curr_dir_hierarchy.begin(), default_dir);
				}
			}
			else
			{
				std::filesystem::path path(s);
				auto str = path.filename().string();
				curr_dir.value = str;
				curr_dir.name = ICON_FA_FOLDER_O" " + str;
			}

			if (file_watcher)
				remove_file_watcher(file_watcher);
			file_watcher = add_file_watcher(FileWatcherModeAll, s);
		}

		void FileSelector::refresh()
		{
			std::string select_dir_filename;
			if (select_dir)
				select_dir_filename = select_dir->filename;

			curr_dir.dir_list.clear();
			curr_dir.file_list.clear();
			select_index = -1;
			select_dir = nullptr;

			if (!on_refresh())
				return;

			std::function<void(DirItem *, const std::filesystem::path &)> fIterDir;
			fIterDir = [&](DirItem *dst, const std::filesystem::path &src) {
				if (src.string() == select_dir_filename)
					select_dir = dst;

				std::filesystem::directory_iterator end_it;
				for (std::filesystem::directory_iterator it(src); it != end_it; it++)
				{
					auto str = it->path().filename().string();
					if (std::filesystem::is_directory(it->status()))
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
						i->value = str;
						i->filename = it->path().string();

						auto ext = it->path().extension().string();
						const char *prefix;
						if (is_text_file(ext))
						{
							i->file_type = FileTypeText;
							prefix = ICON_FA_FILE_TEXT_O" ";
						}
						else if (is_image_file(ext))
						{
							i->file_type = FileTypeImage;
							prefix = ICON_FA_FILE_IMAGE_O" ";
							if (tree_mode)
							{
								i->preview_image = get_texture(i->filename);
								if (!i->preview_image)
									i->preview_image = empty_image;
								else
									increase_texture_ref(i->preview_image.get());
							}
						}
						else if (is_model_file(ext))
						{
							i->file_type = FileTypeModel;
							prefix = ICON_FA_FILE_O" ";
						}
						else if (is_terrain_file(ext))
						{
							i->file_type = FileTypeTerrain;
							prefix = ICON_FA_FILE_O" ";
						}
						else if (is_scene_file(ext))
						{
							i->file_type = FileTypeScene;
							prefix = ICON_FA_FILE_O" ";
						}
						else
							prefix = ICON_FA_FILE_O" ";
						i->name = prefix + str;

						on_add_file_item(i);

						dst->file_list.emplace_back(i);
					}
				}
			};

			fIterDir(&curr_dir, curr_dir.filename);
		}

		void FileSelector::on_show()
		{
			if (file_watcher->dirty)
			{
				file_watcher->dirty = false;
				need_refresh = true;
			}
			if (need_refresh)
			{
				refresh();
				need_refresh = false;
			}

			const float itemSpacing = ImGui::GetStyle().ItemSpacing.x;

			if (!tree_mode)
			{
				ImGui::BeginChild("top", ImVec2(0, ImGui::GetFrameHeightWithSpacing()));

				ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
				auto h_index = 0;
				static int popup_index;
				static std::vector<std::string> popup_list;
				std::string jump_dir;
				for (auto &i : curr_dir_hierarchy)
				{
					ImGui::PushID(h_index);
					if (i != "")
					{
						const auto offset = 5.f;
						ImGui::SetCursorPosY(ImGui::GetCursorPosY() - offset);
						if (ImGui::Button(i.c_str()))
						{
							auto it = curr_dir_hierarchy.begin();
							if (default_dir == "")
								it++;
							for (; ; it++)
							{
								jump_dir += *it;
								if (&i == &(*it))
									break;
								jump_dir += "/";
							}
							if (default_dir == "" && h_index == 1)
								jump_dir += "/";
						}
						ImGui::SameLine();
						ImGui::SetCursorPosY(ImGui::GetCursorPosY() + offset);
					}
					ImGui::SetWindowFontScale(0.5f);
					if (ImGui::Button(ICON_FA_CHEVRON_RIGHT))
					{
						popup_index = -1;
						popup_list.clear();
						if (default_dir == "" && h_index == 0)
						{
							std::filesystem::path path(curr_dir.filename);
							auto root_name = path.root_name().string();
							std::transform(root_name.begin(), root_name.end(), root_name.begin(), tolower);
							for (int i = 0; i < TK_ARRAYSIZE(drivers); i++)
							{
								popup_list.push_back(drivers[i]);
								if (root_name == drivers[i])
									popup_index = i;
							}
						}
						else
						{
							std::string parent_path;
							std::string next_path;
							auto it = curr_dir_hierarchy.begin();
							if (default_dir == "")
								it++;
							for (; ; it++)
							{
								parent_path += *it;
								if (&i == &(*it))
								{
									it++;
									if (it != curr_dir_hierarchy.end())
										next_path = *it;
									break;
								}
								parent_path += "/";
							}
							std::filesystem::directory_iterator end_it;
							int index = 0;
							for (std::filesystem::directory_iterator it(parent_path); it != end_it; it++)
							{
								if (std::filesystem::is_directory(it->status()))
								{
									auto str = it->path().filename().string();
									popup_list.push_back(str);
									if (str == next_path)
										popup_index = index;
									index++;
								}
							}
						}
						ImGui::OpenPopup("DirPopup");
					}
					ImGui::SetWindowFontScale(1.f);
					if (ImGui::BeginPopup("DirPopup"))
					{
						auto index = 0;
						for (auto &s : popup_list)
						{
							if (ImGui::Selectable(s.c_str(), popup_index == index))
							{
								if (default_dir == "" && h_index == 0)
									jump_dir = s + "/";
								else
								{
									auto it = curr_dir_hierarchy.begin();
									if (default_dir == "")
										it++;
									for (; ; it++)
									{
										jump_dir += *it;
										if (&i == &(*it))
											break;
										jump_dir += "/";
									}
									jump_dir += "/" + s;
								}
							}
							index++;
						}
						ImGui::EndPopup();
					}
					ImGui::PopID();
					h_index++;
					if (h_index < curr_dir_hierarchy.size())
						ImGui::SameLine();
				}
				ImGui::PopStyleColor();
				ImGui::PopStyleVar();

				if (jump_dir != "")
					set_current_path(jump_dir);

				ImGui::EndChild();
			}

			if (enable_right_area)
			{
				splitter.set_size_greedily();
				splitter.do_split();
				ImGui::BeginChild("left", ImVec2(splitter.size[0], 0), tree_mode);
			}

			if (!tree_mode)
			{
				on_top_area_show();
				ImGui::BeginChild("list", ImVec2(0, -ImGui::GetFrameHeightWithSpacing() - 1), true);
				auto index = 0;
				if (ImGui::Selectable(ICON_FA_FOLDER_O" ..", select_index == index, ImGuiSelectableFlags_DontClosePopups | ImGuiSelectableFlags_AllowDoubleClick))
				{
					select_index = index;
					if (ImGui::IsMouseDoubleClicked(0))
					{
						std::filesystem::path path(curr_dir.filename);
						if (curr_dir.filename != default_dir && path.root_path() != path)
							set_current_path(path.parent_path().string());
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
							set_current_path((std::filesystem::path(curr_dir.filename) / i->value).string());
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
				auto need_open = select_dir != nullptr;
				std::function<void(DirItem *)> fShowDir;
				fShowDir = [&](DirItem *src) {
					auto node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
					node_flags |= select_dir == src ? ImGuiTreeNodeFlags_Selected : 0;
					if (src->dir_list.empty())
						node_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
					else
					{
						if (select_dir == src)
							need_open = false;
						if (need_open)
							ImGui::SetNextTreeNodeOpen(true);
					}
					auto node_open = ImGui::TreeNodeEx(src->name.c_str(), node_flags);
					if (ImGui::IsItemClicked())
						select_dir = src;
					if (node_open && !(node_flags & ImGuiTreeNodeFlags_Leaf))
					{
						for (auto &d : src->dir_list)
							fShowDir(d.get());
						ImGui::TreePop();
					}
				};
				fShowDir(&curr_dir);
			}

			if (enable_right_area)
			{
				ImGui::EndChild();
				ImGui::SameLine();
				ImGui::BeginChild("right", ImVec2(0, 0));
				on_right_area_show();
				ImGui::EndChild();
			}
		}

		bool FileSelector::on_refresh()
		{
			return true;
		}

		FileSelector::FileItem *FileSelector::on_new_file_item()
		{
			return new FileItem(this);
		}

		void FileSelector::on_add_file_item(FileItem *i)
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
				auto path = std::filesystem::path(curr_dir.filename) / filename;
				if (io_mode == FileSelectorSave || std::filesystem::exists(path))
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
			FileSelector("Dir Selector", FileSelectorOpen, "", WindowModal | WindowNoSavedSettings, FileSelectorNoFiles | FileSelectorNoRightArea)
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
	}
}
