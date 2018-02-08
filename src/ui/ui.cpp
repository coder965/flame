#include <process.h>
#include <map>

#include "../global.h"
#include "../spare_list.h"
#include "../input.h"
#include "../graphics/buffer.h"
#include "../graphics/image.h"
#include "../graphics/renderpass.h"
#include "../graphics/descriptor.h"
#include "../graphics/pipeline.h"
#include "../graphics/sampler.h"
#include "../graphics/command_buffer.h"
#include "../engine.h"

#include "ui.h"

const unsigned int ImageCount = 127;

static tke::SpareList _image_list(ImageCount);
static std::pair<std::shared_ptr<tke::Image>, tke::Op> _image_ops[ImageCount];

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

	ImTextureID ImageID(std::shared_ptr<tke::Image> i)
	{
		auto index = _image_list.add(i.get());
		if (index == -2)
		{
			index = i->ui_index;
			if (_image_ops[index].second == tke::OpNeedRemove)
				_image_ops[index].second = tke::OpKeep;
			return ImTextureID(index + 1);
		}
		else if (index == -1)
		{
			i->ui_index = -1;
			return 0;
		}
		i->ui_index = index;
		_image_ops[index].first = i;
		_image_ops[index].second = tke::OpNeedUpdate;
		return ImTextureID(index + 1);
	}

	void Image_f(const std::string &filename, const ImVec2& size, const ImVec4& border_col)
	{
		auto i = tke::get_image(filename);
		if (!i)
			i = tke::get_image("empty.png");
		assert(i);

		Image(ImageID(i), size, ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, 1), border_col);
	}

	bool ImageButton_f(const std::string &filename, const ImVec2& size, bool active)
	{
		auto i = tke::get_image(filename);
		if (!i)
			i = tke::get_image("empty.png");
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
		ImGuiWindow* window = GetCurrentWindow();
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
			col_bg = ImGui::GetColorU32(ImLerp(ImVec4(0.78f, 0.78f, 0.78f, 1.0f), ImVec4(0.64f, 0.83f, 0.34f, 1.0f), t));
		else
			col_bg = ImGui::GetColorU32(ImLerp(ImVec4(0.85f, 0.85f, 0.85f, 1.0f), ImVec4(0.56f, 0.83f, 0.26f, 1.0f), t));

		draw_list->AddRectFilled(pos, pos + label_size, col_bg, 4);
		RenderText(pos + ImVec2(4, 0), label);

		return pressed;
	}

	static float menubar_height;
	bool BeginMainMenuBar_l()
	{
		auto open = BeginMainMenuBar();
		menubar_height = GetCurrentWindowRead()->Size.y;
		return open;
	}

	static float toolbar_height;
	bool BeginToolBar()
	{
		PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
		toolbar_height = 16.f + GetStyle().WindowPadding.y * 2.f;
		SetNextWindowPos(ImVec2(0, menubar_height));
		SetNextWindowSize(ImVec2(tke::window_cx, toolbar_height));
		PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.8f, 0.91f, 0.94f, 1.f));
		return Begin("toolbar", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings);
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
		statusbar_height = ImGui::GetTextLineHeight() + ImGui::GetStyle().WindowPadding.y * 2.f;
		SetNextWindowPos(ImVec2(0, tke::window_cy - statusbar_height));
		SetNextWindowSize(ImVec2(tke::window_cx, statusbar_height));
		PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.8f, 0.91f, 0.94f, 1.f));
		auto open = ImGui::Begin("statusbar", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings);
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
}

namespace tke
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

		Layout main_layout;

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
				continue_ = _cleanup_layout(&main_layout);
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
			main_layout.width = cx;
			main_layout.height = cy - ImGui::menubar_height - ImGui::toolbar_height - ImGui::statusbar_height;
			_resize_layout(&main_layout);
		}

		Window::Window(const std::string &_title, unsigned int flags) :
			first(true),
			first_cx(0),
			first_cy(0),
			title(_title),
			enable_menu(flags & enable_menu),
			enable_saved_settings(!(flags & WindowNoSavedSettings)),
			modal(flags & WindowModal),
			enable_dock(!(flags & WindowBanDock)),
			opened(true),
			_need_focus(false),
			_tag(WindowTagNull),
			layout(nullptr),
			idx(-1)
		{
			windows.emplace_back(this);
		}

		void Window::dock(Window *w, DockDirection dir)
		{
			if (w == nullptr)
			{
				if (main_layout.type != LayoutCenter)
					return;
				main_layout.add_window(0, this);
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
				if (ori_type != LayoutCenter || ori_idx != 1 - dir)
				{
					for (auto w : ori_layout->windows[ori_idx])
						l->add_window(1 - dir_id, w);
				}
				l->add_window(dir_id, this);
				if (ori_type != LayoutCenter)
					ori_layout->set_layout(ori_idx, l);
				ori_layout->clear_window(ori_idx);
				l->set_size();
			}
		}

		void Window::undock()
		{
			if (!layout)
				return;

			for (auto it = layout->windows[idx].begin(); it != layout->windows[idx].end();)
			{
				if (*it == this)
				{
					auto this_is_curr = false;
					if (layout->curr_tab[idx] == this)
						this_is_curr = true;
					layout->windows[idx].erase(it);
					if (this_is_curr)
					{
						layout->curr_tab[idx] = layout->windows[idx].size() > 0 ?
							layout->windows[idx].back() : nullptr;
					}
					break;
				}
				else
					it++;
			}
			layout = nullptr;
			cleanup_layout();
			on_resize(window_cx, window_cy);
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
			dragging_tab[0] = dragging_tab[1] = nullptr;
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
			curr_tab[idx] = w;
		}

		void Layout::clear_window(int idx)
		{
			windows[idx].clear();
			curr_tab[idx] = nullptr;
		}

		static void _draw_drag_overlay(ImRect rect, Layout *layout, int idx, DockDirection dir)
		{
			if (rect.Contains(ImVec2(mouseX, mouseY)))
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
					if (_rect.Contains(ImVec2(mouseX, mouseY)))
					{
						draw_list->AddRectFilled(rect.Min, rect.Max, col1);
						dock_dir = DockCenter;
					}
				}
				if (dir & DockLeft)
				{
					auto _rect = ImRect(center + ImVec2(-96, -32), center + ImVec2(-64, 32));
					draw_list->AddRectFilled(_rect.Min, _rect.Max, col0);
					if (_rect.Contains(ImVec2(mouseX, mouseY)))
					{
						draw_list->AddRectFilled(rect.Min, rect.Max - ImVec2(rect.GetWidth() / 2.f, 0), col1);
						dock_dir = DockLeft;
					}
				}
				if (dir & DockRight)
				{
					auto _rect = ImRect(center + ImVec2(64, -32), center + ImVec2(96, 32));
					draw_list->AddRectFilled(_rect.Min, _rect.Max, col0);
					if (_rect.Contains(ImVec2(mouseX, mouseY)))
					{
						draw_list->AddRectFilled(rect.Min + ImVec2(rect.GetWidth() / 2.f, 0), rect.Max, col1);
						dock_dir = DockRight;
					}
				}
				if (dir & DockTop)
				{
					auto _rect = ImRect(center + ImVec2(-32, -96), center + ImVec2(32, -64));
					draw_list->AddRectFilled(_rect.Min, _rect.Max, col0);
					if (_rect.Contains(ImVec2(mouseX, mouseY)))
					{
						draw_list->AddRectFilled(rect.Min, rect.Max - ImVec2(0, rect.GetHeight() / 2.f), col1);
						dock_dir = DockTop;
					}
				}
				if (dir & DockBottom)
				{
					auto _rect = ImRect(center + ImVec2(-32, 64), center + ImVec2(32, 96));
					draw_list->AddRectFilled(_rect.Min, _rect.Max, col0);
					if (_rect.Contains(ImVec2(mouseX, mouseY)))
					{
						draw_list->AddRectFilled(rect.Min + ImVec2(0, rect.GetHeight() / 2.f), rect.Max, col1);
						dock_dir = DockBottom;
					}
				}
			}
		}

		void Layout::show_window(int idx)
		{
			auto line_height = ImGui::GetTextLineHeightWithSpacing();
			auto line_height_hf = line_height / 2.f;

			ImGui::BeginChild("##tabbar", ImVec2(0, line_height)); 
			ImU32 color = ImGui::GetColorU32(ImGuiCol_FrameBg);
			ImU32 color_active = ImGui::GetColorU32(ImGuiCol_FrameBgActive);
			ImU32 color_hovered = ImGui::GetColorU32(ImGuiCol_FrameBgHovered);
			ImU32 text_color = ImGui::GetColorU32(ImGuiCol_Text);
			auto index = 0;
			ImVec2 pos = ImGui::GetCursorScreenPos() + ImVec2(line_height_hf, 0);
			for (auto w : windows[idx])
			{
				auto text_end = w->title.c_str() + w->title.size();
				ImVec2 size(ImGui::CalcTextSize(w->title.c_str(), text_end).x, line_height);
				w->tab_x = pos.x;
				w->tab_width = size.x;
				ImGui::SetCursorScreenPos(pos);
				ImGui::InvisibleButton(w->title.c_str(), size);
				auto hovered = false;
				if (ImGui::IsItemHovered())
				{
					hovered = true;
					if (ImGui::IsMouseClicked(0) && dragging_tab[idx] != w)
					{
						dragging_tab[idx] = w;
						dragging_tab_offset[idx] = mouseX - w->tab_x;
					}
					if (ImGui::IsMouseClicked(0))
						curr_tab[idx] = w;
				}

				auto draw_list = ImGui::GetWindowDrawList();
				draw_list->PathClear();

				draw_list->PathArcTo(pos + ImVec2(0, line_height_hf), line_height_hf, glm::radians(90.f), glm::radians(270.f));
				draw_list->PathArcTo(pos + ImVec2(size.x + 9, line_height_hf), line_height_hf, glm::radians(-90.f), glm::radians(90.f));

				draw_list->PathFillConvex(hovered ? color_hovered : (w == curr_tab[idx] ? color_active : color));
				draw_list->AddText(pos, text_color, w->title.c_str(), text_end);

				if (ImGui::CloseButton(ImGui::GetCurrentWindowRead()->GetID((w->title + "#CLOSE").c_str()), pos + ImVec2(size.x + 8, line_height_hf), 5.5f))
					w->_tag = WindowTagClose;

				index++;
				pos.x += size.x + line_height + 9;

				if (index < windows[idx].size())
					ImGui::SameLine();
			}
			ImGui::EndChild();
			if (dragging_tab[idx])
			{
				auto w = dragging_tab[idx];
				if (mouseLeft.pressing)
				{
					for (auto it = windows[idx].begin(); it != windows[idx].end(); it++)
					{
						if (*it == w)
						{
							auto it_ = it;
							auto base = w->tab_x + mouseX - w->tab_x - dragging_tab_offset[idx];
							if (w != windows[idx].front())
							{
								auto t = *(--it_);
								if (base < t->tab_x)
								{
									std::swap(t->tab_x, w->tab_x);
									std::swap(*it_, *it);
									break;
								}
							}
							if (w != windows[idx].back())
							{
								it_ = it;
								auto t = *(++it_);
								if (base + w->tab_width > t->tab_x + t->tab_width)
								{
									std::swap(t->tab_x, w->tab_x);
									std::swap(*it_, *it);
								}
							}
							break;
						}
					}
					if (w->enable_dock && (mouseY < pos.y || mouseY > pos.y + line_height))
						w->_tag = WindowTagUndock;
				}
				else
					dragging_tab[idx] = nullptr;
			}

			ImGui::BeginChild("##content");
			curr_tab[idx]->show();
			ImGui::EndChild();

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
		static CommandBuffer *cb_ui;
		static std::unique_ptr<ImmediateVertexBuffer> vertexBuffer_ui;
		static std::unique_ptr<ImmediateIndexBuffer> indexBuffer_ui;

		static Image *font_image;
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

			pipeline_ui = new Pipeline(PipelineCreateInfo()
				.vertex_input_state({ { TokenF32V2, 0 },{ TokenF32V2, 0 },{ TokenB8V4, 0 } })
				.cull_mode(VK_CULL_MODE_NONE)
				.add_blend_attachment_state(true, VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, VK_BLEND_FACTOR_ZERO, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA)
				.add_dynamic_state(VK_DYNAMIC_STATE_SCISSOR)
				.add_shader(engine_path + "shader/ui.vert", {})
				.add_shader(engine_path + "shader/ui.frag", {}),
				renderPass_window, 0, true);

			{
				ImGuiIO& io = ImGui::GetIO();
				{
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
					font_image = new Image(width, height, VK_FORMAT_R8G8B8A8_UNORM,
						VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 1, 1, false);
					font_image->fill_data(0, pixels, width * height * 4);
					io.Fonts->TexID = (void*)0; // image index

					updateDescriptorSets(&pipeline_ui->descriptorSet->imageWrite(0, 0, font_image, colorSampler));
				}

				cb_ui = new CommandBuffer;
				cb_ui->begin();
				cb_ui->end();

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
			}
		}

		static void _load_layout(XMLNode *n, Layout *layout)
		{
			auto type_name = n->first_attribute("mode")->get_string();
			if (type_name == "horizontal")
				layout->type = LayoutHorizontal;
			else if (type_name == "vertical")
				layout->type = LayoutVertical;
			else
				assert(0); // WIP

			layout->splitter.set_vertically(layout->type == LayoutHorizontal);
			layout->set_size();

			for (int i = 0; i < 2; i++)
			{
				auto c = n->children[i].get();
				if (c->name == "layout")
				{
					auto l = new Layout;
					layout->set_layout(i, l);
					_load_layout(c, l);
				}
				else if (c->name == "window")
				{
					auto window_name = c->first_attribute("name")->get_string();
					for (auto &w : windows)
					{
						if (w->title == window_name)
						{
							layout->add_window(i, w.get());
							break;
						}
					}
				}
				else
					assert(0); // vaild name required
			}
		}

		void begin()
		{
			static int last_time = 0;
			if (last_time == 0) 
				last_time = nowTime;

			accepted_mouse = false;
			accepted_key = false;

			ImGuiIO& io = ImGui::GetIO();

			io.DisplaySize = ImVec2((float)window_cx, (float)window_cy);
			io.DisplayFramebufferScale = ImVec2(1.f, 1.f);

			io.DeltaTime = (float)((nowTime - last_time) / 1000.f);
			last_time = nowTime;

			io.MousePos = ImVec2((float)mouseX, (float)mouseY);

			io.MouseDown[0] = mouseLeft.pressing;
			io.MouseDown[1] = mouseRight.pressing;
			io.MouseDown[2] = mouseMiddle.pressing;

			io.MouseWheel = mouseScroll / 120;

			ImGui::NewFrame();

			static bool first = true;
			if (first)
			{
				on_resize(window_cx, window_cy);
				XMLDoc doc("layout", "ui_layout.xml");
				if (doc.good)
					_load_layout(&doc, &main_layout);
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

			if (!main_layout.is_empty(0))
			{
				ImGui::SetNextWindowPos(ImVec2(0.f, ImGui::menubar_height + ImGui::toolbar_height));
				ImGui::SetNextWindowSize(ImVec2(main_layout.width, main_layout.height));
				ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
				ImGui::Begin("##dock", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | 
					ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus);
				main_layout.show();
				ImGui::End();
				ImGui::PopStyleVar();
			}
			else
			{
				if (dragging_window)
					_draw_drag_overlay(ImRect(0.f, ImGui::menubar_height, main_layout.width, ImGui::menubar_height + ImGui::toolbar_height + main_layout.height), &main_layout, -1, DockCenter);
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
						writes.push_back(pipeline_ui->descriptorSet->imageWrite(0, index + 1, font_image, colorSampler));
					}
					else if (op == OpNeedUpdate)
					{
						auto image = (Image*)p;
						writes.push_back(pipeline_ui->descriptorSet->imageWrite(0, index + 1, image, colorSampler));
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

					size_t vertex_size = draw_data->TotalVtxCount * sizeof(ImDrawVert);
					if (!vertexBuffer_ui || vertexBuffer_ui->size < vertex_size)
						vertexBuffer_ui = std::make_unique<ImmediateVertexBuffer>(vertex_size);

					size_t index_size = draw_data->TotalIdxCount * sizeof(ImDrawIdx);
					if (!indexBuffer_ui || indexBuffer_ui->size < index_size)
						indexBuffer_ui = std::make_unique<ImmediateIndexBuffer>(index_size);

					auto vtx_dst = (ImDrawVert*)vertexBuffer_ui->map(0, vertex_size);
					auto idx_dst = (ImDrawIdx*)indexBuffer_ui->map(0, index_size);
					for (int n = 0; n < draw_data->CmdListsCount; n++)
					{
						const ImDrawList* cmd_list = draw_data->CmdLists[n];
						memcpy(vtx_dst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
						memcpy(idx_dst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));

						vtx_dst += cmd_list->VtxBuffer.Size;
						idx_dst += cmd_list->IdxBuffer.Size;
					}
					vertexBuffer_ui->unmap();
					indexBuffer_ui->unmap();
				}

				cb_ui->reset();
				cb_ui->begin();

				if (bg_color.a > 0.f)
				{
					VkClearValue clear_value = { bg_color.r, bg_color.g, bg_color.b, 1.f };
					cb_ui->beginRenderPass(renderPass_windowC, window_framebuffers[window_imageIndex].get(), &clear_value);
				}
				else
					cb_ui->beginRenderPass(renderPass_window,
						window_framebuffers[window_imageIndex].get());

				if (draw_data->CmdListsCount > 0)
				{
					cb_ui->setViewportAndScissor(window_cx, window_cy);

					cb_ui->bindVertexBuffer(vertexBuffer_ui.get());
					cb_ui->bindIndexBuffer(indexBuffer_ui.get(), VK_INDEX_TYPE_UINT16);

					cb_ui->bindPipeline(pipeline_ui);
					cb_ui->bindDescriptorSet();

					cb_ui->pushConstant(VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::vec4), &glm::vec4(2.f / io.DisplaySize.x, 2.f / io.DisplaySize.y, -1.f, -1.f));

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
								cb_ui->setScissor(ImMax((int32_t)(pcmd->ClipRect.x), 0),
									ImMax((int32_t)(pcmd->ClipRect.y), 0),
									ImMax((uint32_t)(pcmd->ClipRect.z - pcmd->ClipRect.x), 0),
									ImMax((uint32_t)(pcmd->ClipRect.w - pcmd->ClipRect.y + 1), 0)); // TODO: + 1??????
								cb_ui->drawIndex(pcmd->ElemCount, idx_offset, vtx_offset, 1, (int)pcmd->TextureId);
							}
							idx_offset += pcmd->ElemCount;
						}
						vtx_offset += cmd_list->VtxBuffer.Size;
					}
				}

				cb_ui->endRenderPass();

				cb_ui->end();

				add_to_drawlist(cb_ui->v);
			}

			accepted_mouse = ImGui::IsMouseHoveringAnyWindow();
			accepted_key = ImGui::IsAnyItemActive();
		}
	}
}
