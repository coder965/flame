#include <process.h>
#include <map>

#include <flame/global.h>
#include <flame/common/spare_list.h>
#include <flame/common/string.h>
#include <flame/common/system.h>
#include <flame/engine/core/core.h>
#include <flame/engine/graphics/buffer.h>
#include <flame/engine/graphics/texture.h>
#include <flame/engine/graphics/renderpass.h>
#include <flame/engine/graphics/descriptor.h>
#include <flame/engine/graphics/pipeline.h>
#include <flame/engine/graphics/sampler.h>
#include <flame/engine/graphics/command_buffer.h>
#include <flame/engine/core/application.h>
#include <flame/engine/ui/ui.h>

const unsigned int ImageCount = 127;

static flame::SpareList _image_list(ImageCount);
static std::pair<std::shared_ptr<flame::Texture>, flame::Op> _image_ops[ImageCount];

namespace ImGui
{
	const float SplitterThickness = 4.f;

	Splitter::Splitter(bool _vertically, float _min_size1, float _min_size2) :
		vertically(_vertically)
	{
		min_size[0] = size[0] = _min_size1;
		min_size[1] = size[1] = _min_size2;
		set_general_draw_offset();
	}

	void Splitter::set_size_greedily()
	{
		if (vertically)
			size[1] = GetWindowWidth() - GetStyle().ItemSpacing.x - size[0];
		else
			size[1] = GetWindowHeight() - GetStyle().ItemSpacing.y - size[0];
	}

	void Splitter::set_general_draw_offset()
	{
		draw_offset = ((vertically ? ImGui::GetStyle().ItemSpacing.x : ImGui::GetStyle().ItemSpacing.y) - ImGui::SplitterThickness) / 2.f;
	}

	void Splitter::set_vertically(bool _vertically)
	{
		vertically = _vertically;
		set_general_draw_offset();
	}

	bool Splitter::do_split()
	{
		ImGuiContext& g = *GImGui;
		ImGuiWindow* window = g.CurrentWindow;
		ImGuiID id = window->GetID("##Splitter");
		ImRect bb;
		bb.Min = window->DC.CursorPos + (vertically ? ImVec2(size[0] + draw_offset, 0.0f) : ImVec2(0.0f, size[0] + draw_offset));
		bb.Max = bb.Min + CalcItemSize(vertically ? ImVec2(SplitterThickness, -1) : ImVec2(-1, SplitterThickness), 0.0f, 0.0f);
		auto col = ImGui::GetStyleColorVec4(ImGuiCol_Separator);
		col.w = 0.f;
		PushStyleColor(ImGuiCol_Separator, col);
		auto ret = SplitterBehavior(id, bb, vertically ? ImGuiAxis_X : ImGuiAxis_Y, &size[0], &size[1], min_size[0], min_size[1], 0.0f);
		PopStyleColor();
		return ret;
	}

	void TextVFilted(const char* fmt, const char* filter, va_list args)
	{
		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems)
			return;

		ImGuiContext& g = *GImGui;
		const char* text_end = g.TempBuffer + ImFormatStringV(g.TempBuffer, IM_ARRAYSIZE(g.TempBuffer), fmt, args);
		if (filter[0] && !strstr(g.TempBuffer, filter))
			return;
		TextUnformatted(g.TempBuffer, text_end);
	}

	ImTextureID ImageID(std::shared_ptr<flame::Texture> i)
	{
		auto index = _image_list.add(i.get());
		if (index == -2)
		{
			index = i->ui_index;
			if (_image_ops[index].second == flame::OpNeedRemove)
				_image_ops[index].second = flame::OpKeep;
			return ImTextureID(index + 1);
		}
		else if (index == -1)
		{
			i->ui_index = -1;
			return 0;
		}
		i->ui_index = index;
		_image_ops[index].first = i;
		_image_ops[index].second = flame::OpNeedUpdate;
		return ImTextureID(index + 1);
	}

	void Image_f(const std::string &filename, const ImVec2& size, const ImVec4& border_col)
	{
		auto i = flame::get_texture(filename);
		if (!i)
			i = flame::get_texture("empty.png");
		assert(i);

		Image(ImageID(i), size, ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, 1), border_col);
	}

	bool ImageButton_f(const std::string &filename, const ImVec2& size, bool active)
	{
		auto i = flame::get_texture(filename);
		if (!i)
			i = flame::get_texture("empty.png");
		assert(i);

		PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
		if (active)
			PushStyleColor(ImGuiCol_Button, GetColorU32(ImGuiCol_ButtonActive));
		auto pressed = ImageButton(ImageID(i), size, ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0, 0, 0, 0), ImVec4(1, 1, 1, 1));
		if (active)
			PopStyleColor();
		PopStyleColor();
		return pressed;
	}

	bool IconButton(const char *label, float font_scale)
	{
		if (font_scale != 1.f)
			SetWindowFontScale(font_scale);
		PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
		auto pressed = Button(label);
		PopStyleColor();
		if (font_scale != 1.f)
			SetWindowFontScale(1.f);
		return pressed;
	}

	bool Checkbox_2in1(const char *label, bool *v)
	{
		auto window = GetCurrentWindow();
		if (window->SkipItems)
			return false;

		ImVec2 pos = GetCursorScreenPos();
		ImDrawList* draw_list = GetWindowDrawList();

		ImVec2 label_size = CalcTextSize(label, NULL, true);
		label_size.x += 8;

		auto pressed = InvisibleButton(label, label_size);
		if (pressed)
			*v = !*v;

		float t = *v ? 1.0f : 0.0f;

		ImU32 col_bg;
		if (ImGui::IsItemHovered())
			col_bg = ImGui::GetColorU32(*v ? ImVec4(0.64f, 0.83f, 0.34f, 1.0f) : ImVec4(0.78f, 0.78f, 0.78f, 1.0f));
		else
			col_bg = ImGui::GetColorU32(*v ? ImVec4(0.56f, 0.83f, 0.26f, 1.0f) : ImVec4(0.85f, 0.85f, 0.85f, 1.0f));

		draw_list->AddRectFilled(pos, pos + label_size, col_bg, 4);
		RenderText(pos + ImVec2(4, 0), label);

		return pressed;
	}

	static float menubar_height;

	static float toolbar_height;
	bool BeginToolBar()
	{
		PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
		SetNextWindowPos(ImVec2(0, menubar_height));
		SetNextWindowSize(ImVec2(flame::app->window_cx, toolbar_height));
		PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.8f, 0.91f, 0.94f, 1.f));
		return Begin("toolbar", nullptr, ImGuiWindowFlags_NoTitleBar | 
			ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | 
			ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings);
	}

	void EndToolBar()
	{
		End();
		PopStyleColor();
		PopStyleVar();
	}

	static int _statusbar_int_debug;

	static float statusbar_height;
	bool BeginStatusBar()
	{
		PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
		SetNextWindowPos(ImVec2(0, flame::app->window_cy - statusbar_height));
		SetNextWindowSize(ImVec2(flame::app->window_cx, statusbar_height));
		PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.8f, 0.91f, 0.94f, 1.f));
		auto open = ImGui::Begin("statusbar", nullptr, ImGuiWindowFlags_NoTitleBar | 
			ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | 
			ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings);
		Text("%d", _statusbar_int_debug);
		SameLine();
		return open;
	}

	void EndStatusBar()
	{
		End();
		PopStyleColor();
		PopStyleVar();
	}

	void BeginOverlapWindow(const char *title)
	{
		SetNextWindowPos(ImVec2(0, 0));
		SetNextWindowSize(ImVec2(flame::app->window_cx, flame::app->window_cy));
		PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
		Begin(title, nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | 
			ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | 
			ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus);
	}

	void EndOverlapWindow()
	{
		End();
		PopStyleVar();
	}
}

namespace flame
{
	namespace ui
	{
		bool accepted_mouse;
		bool accepted_key;

		const char *get_dock_dir_name(DockDirection dir)
		{
			switch (dir)
			{
				case DockCenter:
					return "center";
				case DockLeft:
					return "left";
				case DockRight:
					return "right";
				case DockTop:
					return "top";
				case DockBottom:
					return "bottom";
			}
			return "null";
		}

		static std::list<std::unique_ptr<Window>> windows;
		static Window *last_dragging_window;
		static Window *dragging_window;
		static Layout *dock_target_layout;
		static int dock_target_idx;
		static DockDirection dock_dir;
		static void set_dragging_window(Window *w)
		{
			if (dragging_window == w)
				return;
			dragging_window = w;
			dock_target_layout = nullptr;
			dock_target_idx = -1;
			dock_dir = (DockDirection)-1;
		}

		Layout *main_layout;

		static bool _cleanup_layout(Layout *l)
		{
			auto dirty = false;
			if (l->is_empty())
			{
				l->type = LayoutCenter;
				return false;
			}
			for (int i = 0; i < 2; i++)
			{
				if (l->is_empty(i))
				{
					auto j = 1 - i;
					if (l->children[j] == nullptr)
					{
						if (j == 1)
						{
							for (auto w : l->windows[1])
								l->add_window(0, w);
						}
						l->clear_window(1);
						l->type = LayoutCenter;
						return false;
					}
					else
					{
						auto c = l->children[j].get();
						l->type = c->type;
						l->width = c->width;
						l->height = c->height;
						for (int k = 0; k < 2; k++)
						{
							l->splitter.size[k] = c->splitter.size[k];
							l->windows[k].clear();
							for (auto w : c->windows[k])
								l->add_window(k, w);
						}
						l->set_layout(i, std::move(c->children[i]));
						l->set_layout(j, std::move(c->children[j]));
					}
					break;
				}
			}
			for (int i = 0; i < 2; i++)
			{
				if (l->children[i])
				{
					auto c = l->children[i].get();
					dirty |= _cleanup_layout(c);
					if (l->children[i]->is_empty())
					{
						l->children[i].reset();
						dirty = true;
					}
					else
					{
						for (int j = 0; j < 2; j++)
						{
							if (!c->children[j] && c->windows[j].size() == 0)
							{
								l->clear_window(i);
								auto k = 1 - j;
								for (auto w : c->windows[k])
									l->add_window(i, w);
								l->children[i].reset();
								dirty = true;
								break;
							}
						}
					}
				}
			}
			return dirty;
		}

		static void cleanup_layout()
		{
			auto continue_ = true;
			while (continue_)
				continue_ = _cleanup_layout(main_layout);
		}

		static void _resize_layout(Layout *l)
		{
			l->set_size();
			for (int i = 0; i < 2; i++)
			{
				if (l->children[i])
					_resize_layout(l->children[i].get());
			}
		}

		static void on_resize(int cx, int cy)
		{
			main_layout->width = cx;
			main_layout->height = cy - ImGui::menubar_height - ImGui::toolbar_height - ImGui::statusbar_height;
			_resize_layout(main_layout);
		}

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

		void Window::dock(Window *w, DockDirection dir)
		{
			if (w == nullptr)
			{
				if (main_layout->type != LayoutCenter)
					return;
				main_layout->add_window(0, this);
				return;
			}

			auto ori_layout = w->layout;
			auto ori_type = ori_layout->type;
			auto ori_idx = w->idx;
			assert(ori_layout);

			if (dir == DockCenter)
				ori_layout->add_window(ori_idx, this);
			else
			{
				auto dir_id = (dir == DockLeft || dir == DockTop) ? 0 : 1;
				auto l = ori_layout;
				if (ori_type != LayoutCenter)
					l = new Layout;
				l->type = (dir == DockLeft || dir == DockRight) ? LayoutHorizontal : LayoutVertical;
				l->splitter.set_vertically(l->type == LayoutHorizontal);
				if (ori_type != LayoutCenter || ori_idx != 1 - dir_id)
				{
					for (auto w : ori_layout->windows[ori_idx])
						l->add_window(1 - dir_id, w);
					ori_layout->clear_window(ori_idx);
				}
				l->add_window(dir_id, this);
				if (ori_type != LayoutCenter)
					ori_layout->set_layout(ori_idx, l);
				l->set_size();
			}
		}

		void Window::undock()
		{
			if (!layout)
				return;

			layout->remove_window(idx, this);
			layout = nullptr;
			cleanup_layout();
			on_resize(app->window_cx, app->window_cy);
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

		FileSelector::FileSelector(const std::string &_title, FileSelectorIo io, const std::string &_default_dir,
			unsigned int window_flags, unsigned int flags) :
			Window(_title, window_flags),
			io_mode(io),
			enable_file(!(flags & FileSelectorNoFiles)),
			enable_right_area(!(flags & FileSelectorNoRightArea)),
			tree_mode(flags & FileSelectorTreeMode),
			splitter(true)
		{
			splitter.size[0] = 300;
			filename[0] = 0;
			if (_default_dir != "")
			{
				default_dir = _default_dir;
				set_current_path(default_dir);
			}
			else
				set_current_path(get_exe_path());
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
			if (file_watcher->ptr->dirty)
			{
				file_watcher->ptr->dirty = false;
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
			return new FileItem;
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

		ImageViewer::ImageViewer(const std::string &_title, std::shared_ptr<Texture> _texture) :
			Window(_title, WindowHasMenu | WindowNoSavedSettings),
			texture(_texture)
		{
			first_cx = 800;
			first_cy = 600;

			staging_buffer = std::make_unique<Buffer>(BufferTypeStaging, texture->get_size());
			texture->copy_to_buffer(staging_buffer.get());
		}

		void ImageViewer::on_show()
		{
			ImGui::BeginMenuBar();
			if (ImGui::BeginMenu("File"))
			{
				auto do_save = false;
				auto need_filename_popup = false;
				if (ImGui::MenuItem("Save"))
				{
					do_save = true;
					if (texture->filename == "")
						need_filename_popup = true;
				}
				if (ImGui::MenuItem("Save As"))
				{
					do_save = true;
					need_filename_popup = true;
				}
				if (do_save)
				{
					auto fun_save = [&](const std::string &filename) {
						staging_buffer->map(0, texture->get_size());
						auto pixel = (unsigned char*)staging_buffer->mapped;
						Image img(texture->get_cx(), texture->get_cy(), texture->channel, texture->bpp, pixel, false);
						img.save(filename);
						staging_buffer->unmap();
						return true;
					};
					if (need_filename_popup)
					{
						auto dialog = new FileSelector("Save Image", FileSelectorSave, "", WindowModal | WindowNoSavedSettings);
						dialog->first_cx = 800;
						dialog->first_cy = 600;
						dialog->callback = fun_save;
					}
					else
						fun_save(texture->filename);
				}
				ImGui::EndMenu();
			}
			on_menu_bar();
			ImGui::EndMenuBar();

			on_top_area();

			auto image_pos = ImGui::GetCursorScreenPos();
			auto image_size = ImVec2(texture->get_cx(), texture->get_cy());
			ImGui::InvisibleButton("canvas", image_size);
			auto draw_list = ImGui::GetWindowDrawList();
			draw_list->AddImage(ImGui::ImageID(texture), image_pos, image_pos + image_size);
			if (ImGui::IsItemHovered())
				on_mouse_overing_image(image_pos);
		}

		float get_layout_padding(bool horizontal)
		{
			if (horizontal)
				return ImGui::GetStyle().WindowPadding.x * 2.f - ImGui::GetStyle().ItemSpacing.x;
			else
				return ImGui::GetStyle().WindowPadding.y * 2.f - ImGui::GetStyle().ItemSpacing.y;
		}

		Layout::Layout() :
			parent(nullptr),
			idx(-1),
			type(LayoutCenter),
			size_radio(0.5f),
			splitter(true)
		{
			splitter.size[0] = splitter.size[1] = -1.f;
			curr_tab[0] = curr_tab[1] = nullptr;
		}

		bool Layout::is_empty(int idx) const
		{
			if (children[idx])
				return false;
			if (windows[idx].size() > 0)
				return false;
			return true;
		}

		bool Layout::is_empty() const
		{
			for (int i = 0; i < 2; i++)
			{
				if (!is_empty(i))
					return false;
			}
			return true;
		}

		void Layout::set_size()
		{
			if (parent)
			{
				if (parent->type == LayoutHorizontal)
				{
					width = parent->splitter.size[idx];
					height = parent->height - get_layout_padding(false);
				}
				else
				{
					width = parent->width - get_layout_padding(true);
					height = parent->splitter.size[idx];
				}
			}
			float s;
			switch (type)
			{
				case LayoutHorizontal:
					s = width - get_layout_padding(true);
					break;
				case LayoutVertical:
					s = height - get_layout_padding(false);
					break;
				default:
					return;
			}
			splitter.size[0] = s * size_radio;
			splitter.size[1] = s * (1.f - size_radio);
		}

		void Layout::set_layout(int idx, Layout *l)
		{
			l->parent = this;
			l->idx = idx;
			children[idx] = std::unique_ptr<Layout>(l);
		}

		void Layout::set_layout(int idx, std::unique_ptr<Layout> &&l)
		{
			if (l)
			{
				l->parent = this;
				l->idx = idx;
			}
			children[idx] = std::move(l);
		}

		void Layout::add_window(int idx, Window *w)
		{
			if (w)
			{
				w->layout = this;
				w->idx = idx;
			}
			windows[idx].push_back(w);
		}

		void Layout::remove_window(int idx, Window *w)
		{
			for (auto it = windows[idx].begin(); it != windows[idx].end(); it++)
			{
				if (*it == w)
				{
					windows[idx].erase(it);
					return;
				}
			}
		}

		void Layout::clear_window(int idx)
		{
			windows[idx].clear();
		}

		static void _draw_drag_overlay(ImRect rect, Layout *layout, int idx, DockDirection dir)
		{
			if (rect.Contains(ImVec2(app->mouseX, app->mouseY)))
			{
				dock_target_layout = layout;
				dock_target_idx = idx;
				auto draw_list = ImGui::GetOverlayDrawList();
				auto center = rect.GetCenter();
				ImColor col0(0.7f, 0.1f, 1.f, 0.5f);
				ImColor col1(0.3f, 0.2f, 0.5f, 0.5f);
				if (dir & DockCenter)
				{
					auto _rect = ImRect(center + ImVec2(-32, -32), center + ImVec2(32, 32));
					draw_list->AddRectFilled(_rect.Min, _rect.Max, col0);
					if (_rect.Contains(ImVec2(app->mouseX, app->mouseY)))
					{
						draw_list->AddRectFilled(rect.Min, rect.Max, col1);
						dock_dir = DockCenter;
					}
				}
				if (dir & DockLeft)
				{
					auto _rect = ImRect(center + ImVec2(-96, -32), center + ImVec2(-64, 32));
					draw_list->AddRectFilled(_rect.Min, _rect.Max, col0);
					if (_rect.Contains(ImVec2(app->mouseX, app->mouseY)))
					{
						draw_list->AddRectFilled(rect.Min, rect.Max - ImVec2(rect.GetWidth() / 2.f, 0), col1);
						dock_dir = DockLeft;
					}
				}
				if (dir & DockRight)
				{
					auto _rect = ImRect(center + ImVec2(64, -32), center + ImVec2(96, 32));
					draw_list->AddRectFilled(_rect.Min, _rect.Max, col0);
					if (_rect.Contains(ImVec2(app->mouseX, app->mouseY)))
					{
						draw_list->AddRectFilled(rect.Min + ImVec2(rect.GetWidth() / 2.f, 0), rect.Max, col1);
						dock_dir = DockRight;
					}
				}
				if (dir & DockTop)
				{
					auto _rect = ImRect(center + ImVec2(-32, -96), center + ImVec2(32, -64));
					draw_list->AddRectFilled(_rect.Min, _rect.Max, col0);
					if (_rect.Contains(ImVec2(app->mouseX, app->mouseY)))
					{
						draw_list->AddRectFilled(rect.Min, rect.Max - ImVec2(0, rect.GetHeight() / 2.f), col1);
						dock_dir = DockTop;
					}
				}
				if (dir & DockBottom)
				{
					auto _rect = ImRect(center + ImVec2(-32, 64), center + ImVec2(32, 96));
					draw_list->AddRectFilled(_rect.Min, _rect.Max, col0);
					if (_rect.Contains(ImVec2(app->mouseX, app->mouseY)))
					{
						draw_list->AddRectFilled(rect.Min + ImVec2(0, rect.GetHeight() / 2.f), rect.Max, col1);
						dock_dir = DockBottom;
					}
				}
			}
		}

		void Layout::show_window(int idx)
		{
			ImGui::BeginTabBar("tabbar");
			for (auto &w : windows[idx])
			{
				auto open = true;
				auto curr = ImGui::TabItem(w->title.c_str(), &open);
				if (ImGui::IsItemActive())
				{
					auto mouseY = ImGui::GetMousePos().y;
					if (mouseY < ImGui::GetItemRectMin().y || mouseY > ImGui::GetItemRectMax().y)
						w->_tag = WindowTagUndock;
				}
				if (curr)
				{
					curr_tab[idx] = w;
					w->show();
				}
				if (!open)
					w->_tag = WindowTagClose;
			}
			ImGui::EndTabBar();

			if (dragging_window)
			{
				auto pos = ImGui::GetWindowPos();
				ImRect window_rect(pos, pos + ImGui::GetWindowSize());
				_draw_drag_overlay(ImRect(pos, pos + ImGui::GetWindowSize()), this, idx, DockAll);
			}
		}

		void Layout::show()
		{
			switch (type)
			{
				case LayoutCenter:
					show_window(0);
					break;
				case LayoutHorizontal: case LayoutVertical:
				{
					splitter.do_split();
					splitter.set_size_greedily();
					size_radio = splitter.size[0] / (splitter.size[0] + splitter.size[1]);
					for (int i = 0; i < 2; i++)
					{
						ImGui::PushID(i);
						ImGui::BeginChild("##part", type == LayoutHorizontal ? ImVec2(splitter.size[i], 0) : ImVec2(0, splitter.size[i]), false);
						if (children[i])
							children[i]->show();
						else
							show_window(i);
						ImGui::EndChild();
						ImGui::PopID();
						if (i == 0 && type == LayoutHorizontal)
							ImGui::SameLine();
					}
					break;
				}
			}
		}

		glm::vec4 bg_color = glm::vec4(0.35f, 0.57f, 0.1f, 1.f);

		glm::vec4 get_bg_color()
		{
			return bg_color;
		}

		void set_bg_color(const glm::vec4 &v)
		{
			bg_color = v;
		}

		static Pipeline *pipeline_ui;
		static Pipeline *pipeline_sdf_text;
		static const char *sdf_text_chars = " 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
		static int sdf_text_char_count = strlen(sdf_text_chars);
		static int sdf_text_size = 32;
		struct SdfTextDrawCommand
		{
			std::string text;
			int x;
			int y;
			int size;
		};
		struct SdfTextDrawVertex
		{
			glm::vec2 pos;
			glm::vec2 uv;
		};
		static std::vector<SdfTextDrawCommand> sdf_text_draw_commands;
		static std::unique_ptr<Buffer> sdf_text_vertex_buffer;
		static CommandBuffer *cmd;
		static std::unique_ptr<Buffer> vertexBuffer_ui;
		static std::unique_ptr<Buffer> indexBuffer_ui;

		static Texture *font_image;
		static Texture *sdf_font_image;

		void init()
		{
			add_keydown_listener([](int k) {
				ImGuiIO& io = ImGui::GetIO();
				io.KeysDown[k] = true;

				io.KeyCtrl = io.KeysDown[VK_LCONTROL] || io.KeysDown[VK_RCONTROL] || io.KeysDown[VK_CONTROL];
				io.KeyShift = io.KeysDown[VK_LSHIFT] || io.KeysDown[VK_RSHIFT] || io.KeysDown[VK_SHIFT];
				io.KeyAlt = io.KeysDown[VK_LMENU] || io.KeysDown[VK_RMENU];
				io.KeySuper = io.KeysDown[VK_LWIN] || io.KeysDown[VK_RWIN];
			});

			add_keyup_listener([](int k) {
				ImGuiIO& io = ImGui::GetIO();
				io.KeysDown[k] = false;

				io.KeyCtrl = io.KeysDown[VK_LCONTROL] || io.KeysDown[VK_RCONTROL] || io.KeysDown[VK_CONTROL];
				io.KeyShift = io.KeysDown[VK_LSHIFT] || io.KeysDown[VK_RSHIFT] || io.KeysDown[VK_SHIFT];
				io.KeyAlt = io.KeysDown[VK_LMENU] || io.KeysDown[VK_RMENU];
				io.KeySuper = io.KeysDown[VK_LWIN] || io.KeysDown[VK_RWIN];
			});

			add_char_listener([](int c) {
				if (c == VK_TAB)
					return;

				ImGuiIO& io = ImGui::GetIO();
				if (c > 0 && c < 0x10000)
					io.AddInputCharacter((unsigned short)c);
			});

			pipeline_ui = new Pipeline(PipelineInfo()
				.set_vertex_input_state({ { TokenF32V2, 0 },{ TokenF32V2, 0 },{ TokenB8V4, 0 } })
				.set_cull_mode(VK_CULL_MODE_NONE)
				.add_blend_attachment_state(true, VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, 
					VK_BLEND_FACTOR_ZERO, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA)
				.add_dynamic_state(VK_DYNAMIC_STATE_SCISSOR)
				.add_shader("ui.vert", {})
				.add_shader("ui.frag", {}),
				renderPass_window, 0, true);

			pipeline_sdf_text = new Pipeline(PipelineInfo()
				.set_vertex_input_state({ { TokenF32V2, 0 },{ TokenF32V2, 0 } })
				.set_cull_mode(VK_CULL_MODE_NONE)
				.add_blend_attachment_state(true, VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
					VK_BLEND_FACTOR_ZERO, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA)
				.add_shader("sdf_text.vert", {})
				.add_shader("sdf_text.frag", {}),
				renderPass_window, 0, true);

			ImGui::CreateContext();
			ImGuiIO& io = ImGui::GetIO();

			//io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/msmincho.ttc", 16, nullptr, io.Fonts->GetGlyphRangesJapanese());
			io.Fonts->AddFontDefault();
			static const ImWchar icons_ranges[] = {
				ICON_MIN_FA,
				ICON_MAX_FA,
				0
			};
			ImFontConfig icons_config;
			icons_config.MergeMode = true;
			icons_config.PixelSnapH = true;
			io.Fonts->AddFontFromFileTTF("icon.ttf", 16.0f, &icons_config, icons_ranges);
			unsigned char* pixels; int width, height;
			io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
			font_image = new Texture(width, height, VK_FORMAT_R8G8B8A8_UNORM,
				VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 1, 1, false);
			font_image->fill_data(0, pixels);
			io.Fonts->TexID = (void*)0; // image index
			updateDescriptorSets(&pipeline_ui->descriptor_set->imageWrite(0, 0, font_image, colorSampler));

			if (!std::filesystem::exists("sdf.rimg"))
			{
				std::string cl(sdf_text_chars);
				cl += " ";
				cl += std::to_string(sdf_text_size);
				exec("sdf_generator", cl.c_str());
			}

			Image sdf("sdf.rimg", true);
			sdf.add_alpha_channel();

			sdf_font_image = new Texture(sdf_text_size * sdf_text_char_count, sdf_text_size, VK_FORMAT_R8G8B8A8_UNORM,
				VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 1, 1, false);
			sdf_font_image->fill_data(0, sdf.data);
			updateDescriptorSets(&pipeline_sdf_text->descriptor_set->imageWrite(0, 0, sdf_font_image, colorSampler));

			cmd = new CommandBuffer;

			io.KeyMap[ImGuiKey_Tab] = VK_TAB;
			io.KeyMap[ImGuiKey_LeftArrow] = VK_LEFT;
			io.KeyMap[ImGuiKey_RightArrow] = VK_RIGHT;
			io.KeyMap[ImGuiKey_UpArrow] = VK_UP;
			io.KeyMap[ImGuiKey_DownArrow] = VK_DOWN;
			io.KeyMap[ImGuiKey_PageUp] = VK_PRIOR;
			io.KeyMap[ImGuiKey_PageDown] = VK_NEXT;
			io.KeyMap[ImGuiKey_Home] = VK_HOME;
			io.KeyMap[ImGuiKey_End] = VK_END;
			io.KeyMap[ImGuiKey_Delete] = VK_DELETE;
			io.KeyMap[ImGuiKey_Backspace] = VK_BACK;
			io.KeyMap[ImGuiKey_Enter] = VK_RETURN;
			io.KeyMap[ImGuiKey_Escape] = VK_ESCAPE;
			io.KeyMap[ImGuiKey_A] = 'A';
			io.KeyMap[ImGuiKey_C] = 'C';
			io.KeyMap[ImGuiKey_V] = 'V';
			io.KeyMap[ImGuiKey_X] = 'X';
			io.KeyMap[ImGuiKey_Y] = 'Y';
			io.KeyMap[ImGuiKey_Z] = 'Z';
			io.SetClipboardTextFn = [](void *user_data, const char *s) {
				set_clipBoard(s);
			};
			io.GetClipboardTextFn = [](void *user_data) {
				static std::string s;
				s = get_clipBoard();
				return s.c_str();
			};

			ImGui::StyleColorsLight(nullptr);

			main_layout = new Layout;
		}

		static void _load_layout(XMLNode *n, Layout *layout)
		{
			auto mode_name = n->first_attribute("mode")->get_string();
			if (mode_name == "horizontal")
				layout->type = LayoutHorizontal;
			else if (mode_name == "vertical")
				layout->type = LayoutVertical;
			else if (mode_name == "center")
				layout->type = LayoutCenter;
			else
				assert(0); // vaild name required

			layout->size_radio = n->first_attribute("size_radio")->get_float();
			layout->splitter.set_vertically(layout->type == LayoutHorizontal);
			layout->set_size();

			for (int i = 0; i < 2; i++)
			{
				auto c = n->children[i].get();
				if (c->name == "node")
				{
					auto type_name = c->first_attribute("type")->get_string();
					if (type_name == "layout")
					{
						auto l = new Layout;
						layout->set_layout(i, l);
						_load_layout(c, l);
					}
					else if (type_name == "windows")
					{
						for (auto &cc : c->children)
						{
							if (cc->name == "window")
							{
								auto window_name = cc->first_attribute("name")->get_string();
								for (auto &w : windows)
								{
									if (w->title == window_name)
									{
										layout->add_window(i, w.get());
										break;
									}
								}
							}
						}
					}
					else
						assert(0); // vaild name required
				}
				else
					assert(0); // vaild name required
			}
		}

		void begin()
		{
			accepted_mouse = false;
			accepted_key = false;

			ImGuiIO& io = ImGui::GetIO();

			io.DisplaySize = ImVec2((float)app->window_cx, (float)app->window_cy);
			io.DisplayFramebufferScale = ImVec2(1.f, 1.f);

			io.DeltaTime = elapsed_time;

			io.MousePos = ImVec2((float)app->mouseX, (float)app->mouseY);

			for (auto i = 0; i < 3; i++)
				io.MouseDown[i] = app->mouse_button[i].pressing;

			io.MouseWheel = app->mouseScroll / 120;

			ImGui::NewFrame();

			static bool first = true;
			if (first)
			{
				ImGuiContext& g = *GImGui;
				ImGui::menubar_height = g.FontBaseSize + g.Style.FramePadding.y * 2.0f;
				ImGui::toolbar_height = 16.f + g.Style.WindowPadding.y * 2.f;
				ImGui::statusbar_height = ImGui::GetTextLineHeight() + g.Style.WindowPadding.y * 2.f;

				on_resize(app->window_cx, app->window_cy);
				XMLDoc doc("layout", "ui_layout.xml");
				if (doc.good)
					_load_layout(&doc, main_layout);
				cleanup_layout();
				add_resize_listener(on_resize);

				first = false;
			}

			_image_list.iterate([&](int index, void *p, bool &remove) {
				_image_ops[index].second = OpNeedRemove;
				return true;
			});
		}

		void end()
		{
			last_dragging_window = dragging_window;
			dragging_window = nullptr;

			for (auto &w : windows)
			{
				if (w->_tag == WindowTagClose)
				{
					w->undock();
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
					w->undock();
				if (!w->layout)
					w->show();
				w->_tag = WindowTagNull;
			}

			if (!main_layout->is_empty(0))
			{
				ImGui::SetNextWindowPos(ImVec2(0.f, ImGui::menubar_height + ImGui::toolbar_height));
				ImGui::SetNextWindowSize(ImVec2(main_layout->width, main_layout->height));
				ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
				ImGui::Begin("##dock", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | 
					ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus);
				main_layout->show();
				ImGui::End();
				ImGui::PopStyleVar();
			}
			else
			{
				if (dragging_window)
					_draw_drag_overlay(ImRect(0.f, ImGui::menubar_height, main_layout->width, ImGui::menubar_height + ImGui::toolbar_height + main_layout->height), main_layout, -1, DockCenter);
			}

			if (last_dragging_window != dragging_window)
			{
				if (last_dragging_window && dock_target_layout && dock_dir != -1)
					last_dragging_window->dock(dock_target_layout->curr_tab[dock_target_idx], dock_dir);
			}

			{
				std::vector<VkWriteDescriptorSet> writes;
				_image_list.iterate([&](int index, void *p, bool &remove) {
					auto op = _image_ops[index].second;
					if (op == OpNeedRemove)
					{
						remove = true;
						_image_ops[index].first.reset();
						writes.push_back(pipeline_ui->descriptor_set->imageWrite(0, index + 1, font_image, colorSampler));
					}
					else if (op == OpNeedUpdate)
					{
						auto image = (Texture*)p;
						writes.push_back(pipeline_ui->descriptor_set->imageWrite(0, index + 1, image, colorSampler));
					}
					return true;
				});
				updateDescriptorSets(writes.size(), writes.data());
			}

			ImGui::Render();

			ImGuiIO& io = ImGui::GetIO();
			if ((int)(io.DisplaySize.x * io.DisplayFramebufferScale.x) > 0 && (int)(io.DisplaySize.y * io.DisplayFramebufferScale.y) > 0)
			{
				auto draw_data = ImGui::GetDrawData();
				if (draw_data->CmdListsCount > 0)
				{
					draw_data->ScaleClipRects(io.DisplayFramebufferScale);

					auto vertex_size = draw_data->TotalVtxCount * sizeof(ImDrawVert);
					if (!vertexBuffer_ui || vertexBuffer_ui->size < vertex_size)
					{
						if (vertexBuffer_ui)
							vertexBuffer_ui->unmap();
						vertexBuffer_ui = std::make_unique<Buffer>(BufferTypeImmediateVertex, vertex_size);
						vertexBuffer_ui->map();
					}

					auto index_size = draw_data->TotalIdxCount * sizeof(ImDrawIdx);
					if (!indexBuffer_ui || indexBuffer_ui->size < index_size)
					{
						if (indexBuffer_ui)
							indexBuffer_ui->unmap();
						indexBuffer_ui = std::make_unique<Buffer>(BufferTypeImmediateIndex, index_size);
						indexBuffer_ui->map();
					}

					auto vtx_dst = (ImDrawVert*)vertexBuffer_ui->mapped;
					auto idx_dst = (ImDrawIdx*)indexBuffer_ui->mapped;

					for (int n = 0; n < draw_data->CmdListsCount; n++)
					{
						const auto cmd_list = draw_data->CmdLists[n];
						memcpy(vtx_dst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
						memcpy(idx_dst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
						vtx_dst += cmd_list->VtxBuffer.Size;
						idx_dst += cmd_list->IdxBuffer.Size;
					}

					vertexBuffer_ui->flush();
					indexBuffer_ui->flush();
				}

				auto _chr_count = 0;
				if (!sdf_text_draw_commands.empty())
				{
					for (auto &c : sdf_text_draw_commands)
					{
						for (auto i = 0; i < c.text.size(); i++)
						{
							auto chr = c.text[i];
							if (chr == ' ' ||
								(chr >= '0' && chr <= '9') ||
								(chr >= 'A' && chr <= 'Z') || 
								(chr >= 'a' && chr <= 'z'))
								_chr_count++;
						}
					}

					auto sdf_text_vertex_size = _chr_count * 6 * sizeof(SdfTextDrawVertex);
					if (!sdf_text_vertex_buffer || sdf_text_vertex_buffer->size < sdf_text_vertex_size)
						sdf_text_vertex_buffer = std::make_unique<Buffer>(BufferTypeImmediateVertex, sdf_text_vertex_size);

					sdf_text_vertex_buffer->map(0, sdf_text_vertex_size);
					auto vtx_dst = (SdfTextDrawVertex*)sdf_text_vertex_buffer->mapped;
					for (auto &cmd : sdf_text_draw_commands)
					{
						auto _chr_count = 0;
						for (auto i = 0; i < cmd.text.size(); i++)
						{
							auto chr = cmd.text[i];
							int offset;
							if (chr == ' ')
								offset = 0;
							else if (chr >= '0' && chr <= '9')
								offset = chr - '0' + 1;
							else if (chr >= 'A' && chr <= 'Z')
								offset = chr - 'A' + 1 + 10;
							else if (chr >= 'a' && chr <= 'z')
								offset = chr - 'a' + 1 + 10 + 26;
							else
								continue;
							auto w_s = glm::vec2(app->window_cx, app->window_cy);
							auto hs = glm::vec2(cmd.size, cmd.size) / w_s / 2.f;
							auto p = glm::vec2(cmd.x + _chr_count * cmd.size, cmd.y) / w_s;
							auto a_pos = p - hs;
							a_pos = a_pos * 2.f - 1.f;
							auto b_pos = p + glm::vec2(hs.x, -hs.y);
							b_pos = b_pos * 2.f - 1.f;
							auto c_pos = p + glm::vec2(-hs.x, hs.y);
							c_pos = c_pos * 2.f - 1.f;
							auto d_pos = p + hs;
							d_pos = d_pos * 2.f - 1.f;
							auto u0 = (float)offset / sdf_text_char_count;
							auto u1 = (float)(offset + 1) / sdf_text_char_count;
							vtx_dst[0].pos = a_pos;
							vtx_dst[0].uv = glm::vec2(u0, 0.f);
							vtx_dst[1].pos = c_pos;
							vtx_dst[1].uv = glm::vec2(u0, 1.f);
							vtx_dst[2].pos = d_pos;
							vtx_dst[2].uv = glm::vec2(u1, 1.f);
							vtx_dst[3].pos = a_pos;
							vtx_dst[3].uv = glm::vec2(u0, 0.f);
							vtx_dst[4].pos = d_pos;
							vtx_dst[4].uv = glm::vec2(u1, 1.f);
							vtx_dst[5].pos = b_pos;
							vtx_dst[5].uv = glm::vec2(u1, 0.f);
							vtx_dst += 6;
							_chr_count++;
						}
					}
					sdf_text_vertex_buffer->unmap();
				}

				cmd->begin();

				if (main_layout->is_empty(0))
				{
					VkClearValue clear_value = { bg_color.r, bg_color.g, bg_color.b, 1.f };
					cmd->begin_renderpass(renderPass_windowC, app->get_curr_framebuffer(), &clear_value);
				}
				else
					cmd->begin_renderpass(renderPass_window, app->get_curr_framebuffer());

				if (draw_data->CmdListsCount > 0)
				{
					cmd->set_viewport_and_scissor(app->window_cx, app->window_cy);

					cmd->bind_vertex_buffer(vertexBuffer_ui.get());
					cmd->bind_index_buffer(indexBuffer_ui.get(), VK_INDEX_TYPE_UINT16);

					cmd->bind_pipeline(pipeline_ui);
					cmd->bind_descriptor_set();

					cmd->push_constant(VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::vec4), &glm::vec4(2.f / io.DisplaySize.x, 2.f / io.DisplaySize.y, -1.f, -1.f));

					int vtx_offset = 0;
					int idx_offset = 0;
					for (int n = 0; n < draw_data->CmdListsCount; n++)
					{
						const ImDrawList* cmd_list = draw_data->CmdLists[n];
						for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
						{
							const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
							if (pcmd->UserCallback)
							{
								pcmd->UserCallback(cmd_list, pcmd);
								pcmd->TextureId;
							}
							else
							{
								cmd->set_scissor(
									ImMax((int32_t)(pcmd->ClipRect.x), 0),
									ImMax((int32_t)(pcmd->ClipRect.y), 0),
									ImMax((uint32_t)(pcmd->ClipRect.z - pcmd->ClipRect.x), 0),
									ImMax((uint32_t)(pcmd->ClipRect.w - pcmd->ClipRect.y + 1), 0)  // TODO: + 1??????
								);
								cmd->draw_index(pcmd->ElemCount, idx_offset, vtx_offset, 1, (int)pcmd->TextureId);
							}
							idx_offset += pcmd->ElemCount;
						}
						vtx_offset += cmd_list->VtxBuffer.Size;
					}
				}

				if (!sdf_text_draw_commands.empty())
				{
					cmd->set_scissor(0, 0, app->window_cx, app->window_cy);

					cmd->bind_vertex_buffer(sdf_text_vertex_buffer.get());

					cmd->bind_pipeline(pipeline_sdf_text);
					cmd->bind_descriptor_set();

					cmd->draw(_chr_count * 6);

					sdf_text_draw_commands.clear();
				}

				cmd->end_renderpass();

				cmd->end();

				app->add_cb(cmd->v);
			}

			accepted_mouse = ImGui::IsMouseHoveringAnyWindow();
			accepted_key = ImGui::IsAnyItemActive();
		}

		static void _save_layout(XMLNode *n, Layout *layout)
		{
			std::string mode_name;
			switch (layout->type)
			{
				case LayoutHorizontal:
					mode_name = "horizontal";
					break;
				case LayoutVertical:
					mode_name = "vertical";
					break;
				case LayoutCenter:
					mode_name = "center";
					break;
			}
			n->add_attribute(new XMLAttribute("mode", mode_name));

			n->add_attribute(new XMLAttribute("size_radio", layout->size_radio));

			for (int i = 0; i < 2; i++)
			{
				auto c = new XMLNode("node");
				n->add_node(c);
				if (layout->children[i])
				{
					c->add_attribute(new XMLAttribute("type", "layout"));
					_save_layout(c, layout->children[i].get());
				}
				else
				{
					c->add_attribute(new XMLAttribute("type", "windows"));
					for (auto w : layout->windows[i])
					{
						auto window_node = new XMLNode("window");
						window_node->add_attribute(new XMLAttribute("name", w->title));
						c->add_node(window_node);
					}
				}
			}
		}

		void draw_text(const std::string &text, int x, int y, int size)
		{
			sdf_text_draw_commands.push_back({text, x, y, size});
		}

		void save_layout()
		{
			XMLDoc doc("layout");
			_save_layout(&doc, main_layout);
			doc.save("ui_layout.xml");
		}
	}
}
