//MIT License
//
//Copyright (c) 2018 wjs
//
//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files (the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions:
//
//The above copyright notice and this permission notice shall be included in all
//copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//SOFTWARE.

#include <flame/surface.h>
#include <flame/system.h>
#include <flame/filesystem.h>
#include <flame/math.h>
#include <flame/graphics/device.h>
#include <flame/graphics/swapchain.h>
#include <flame/graphics/renderpass.h>
#include <flame/graphics/shader.h>
#include <flame/graphics/pipeline.h>
#include <flame/graphics/framebuffer.h>
#include <flame/graphics/commandbuffer.h>
#include <flame/graphics/semaphore.h>
#include <flame/graphics/queue.h>
#include <flame/UI/instance.h>

#define NOMINMAX
#include <Windows.h>

int main(int argc, char **args)
{
	using namespace flame;
	using namespace glm;

	vec2 res(1280, 720);

	auto sm = create_surface_manager();
	auto s = sm->create_surface(res.x, res.y, SurfaceStyleFrame,
		"UI Editor");

	auto d = graphics::create_device(false);

	auto sc = graphics::create_swapchain(d, s->get_win32_handle(), s->cx, s->cy);

	auto rp_ui = graphics::create_renderpass(d);
	rp_ui->add_attachment(sc->format, true);
	rp_ui->add_subpass({0}, -1);
	rp_ui->build();

	graphics::Framebuffer *fbs_ui[2];
	for (auto i = 0; i < 2; i++)
	{
		fbs_ui[i] = create_framebuffer(d, res.x, res.y, rp_ui);
		fbs_ui[i]->set_view_swapchain(0, sc, i);
		fbs_ui[i]->build();
	}
	auto cb_ui = d->cp->create_commandbuffer();
	cb_ui->begin();
	cb_ui->end();

	auto image_avalible = graphics::create_semaphore(d);
	auto ui_finished = graphics::create_semaphore(d);

	auto ui = UI::create_instance(d, rp_ui, s);

	MediumString wnd_name;
	strcpy(wnd_name.data, "New Form");
	vec2 wnd_pos{ 50.f, 20.f };
	vec2 wnd_size{ 300.f, 400.f };

	enum WidgetType
	{
		WidgetText,
		WidgetButton
	};

	struct Widget
	{
		vec4 rect;

		WidgetType type;
		ShortString name;
		unsigned int ID;

		virtual void show(UI::Instance *ui) = 0;
	};

	struct TextWidget : Widget
	{
		MediumString text;

		virtual void show(UI::Instance *ui) override
		{
			ui->ID_text_unformatted(name.data, text.data);
		}
	};

	struct ButtonWidget : Widget
	{
		virtual void show(UI::Instance *ui) override
		{
			ui->button(name.data);
		}
	};

	std::vector<std::unique_ptr<Widget>> widgets;

	auto find_widget = [&](const char *name) {
		for (auto &w : widgets)
		{
			if (strcmp(w->name.data, name) == 0)
				return true;
		}
		return false;
	};

	sm->run([&](){
		ui->begin(res.x, res.y, sm->elapsed_time);

		/*
		0xFFFFFFFF:selecting nothing
		nullptr:selecting the window
		else:selecting a widget
		*/
		static Widget *sel = (Widget*)0xFFFFFFFF;
		vec4 sel_rect;

		static bool transform_mode = false;
		static bool transform_mode_moving = false;
		static bool transform_mode_sizing = false;
		static CursorType transform_mode_cursor;
		static ivec2 transform_mode_anchor;
		static RectSide transform_mode_sizing_side;
		auto toggle_transform_mode = []() {
			if (transform_mode)
				transform_mode = false;
			else
			{
				if (sel != (Widget*)0xFFFFFFFF)
				{
					transform_mode = true;
					transform_mode_moving = false;
					transform_mode_sizing = false;
				}
			}
		};

		static bool hierarchy = false;
		bool hierarchy_need_set_to_center = false;
		static bool inspector = false;
		bool inspector_need_set_to_center = false;

		ui->begin_mainmenu();
		if (ui->begin_menu("File"))
		{
			if (ui->menuitem("New", "Ctrl+N"))
			{
			}
			if (ui->menuitem("Open", "Ctrl+O"))
			{
			}
			if (ui->menuitem("Save", "Ctrl+S"))
			{
			}
			ui->end_menu();
		}
		if (ui->begin_menu("Add"))
		{
			if (ui->menuitem("Text"))
			{
				std::string default_name;
				for (auto i = 0; ; i++)
				{
					default_name = "Text" + std::to_string(i);
					if (!find_widget(default_name.c_str()))
						break;
				}
				ui->add_input_dialog("Please Enter The ID Of Text", "ID", [&](MediumString *input) {
					if (input->data[0] == 0)
					{
						ui->add_message_dialog("Add Text", "ID cannot be empty");
						return;
					}
					if (find_widget(input->data))
					{
						ui->add_message_dialog("Add Text", "This ID already exists");
						return;
					}
					auto w = new TextWidget;
					w->type = WidgetText;
					strcpy(w->name.data, input->data);
					strcpy(w->text.data, input->data);
					widgets.emplace_back(w);
				}, default_name.c_str());
			}
			if (ui->menuitem("Button"))
			{
				std::string default_name;
				for (auto i = 0; ; i++)
				{
					default_name = "Button" + std::to_string(i);
					if (!find_widget(default_name.c_str()))
						break;
				}
				ui->add_input_dialog("Please Enter The ID Of Button", "ID", [&](MediumString *input) {
					if (input->data[0] == 0)
					{
						ui->add_message_dialog("Add Button", "ID cannot be empty");
						return;
					}
					if (find_widget(input->data))
					{
						ui->add_message_dialog("Add Button", "This ID already exists");
						return;
					}
					auto w = new ButtonWidget;
					w->type = WidgetButton;
					strcpy(w->name.data, input->data);
					widgets.emplace_back(w);
				}, default_name.c_str());
			}
			ui->end_menu();
		}
		if (ui->begin_menu("Edit"))
		{
			if (ui->menuitem("Undo", "Ctrl+Z"))
				;
			if (ui->menuitem("Redo", "Ctrl+Y"))
				;
			if (ui->menuitem("Cut", "Ctrl+X"))
				;
			if (ui->menuitem("Copy", "Ctrl+C"))
				;
			if (ui->menuitem("Paste", "Ctrl+V"))
				;
			if (ui->menuitem("Delete", "Del"))
				;
			if (ui->menuitem("Transform Mode", "T", transform_mode))
				toggle_transform_mode();
			ui->end_menu();
		}
		if (ui->begin_menu("View"))
		{
			if (ui->menuitem("Hierarchy", "H", hierarchy))
				;
			if (ui->menuitem("Inspector", "I", inspector))
				;
			ui->end_menu();
		}
		auto menu_rect = ui->get_curr_window_rect();
		ui->end_mainmenu();

		auto transform_mode_move_off = ivec2(s->mouse_x - transform_mode_anchor.x, 
			s->mouse_y - transform_mode_anchor.y);
		auto transform_mode_size_off = ivec2(s->mouse_x - transform_mode_anchor.x,
			s->mouse_y - transform_mode_anchor.y);
		switch (transform_mode_sizing_side)
		{
		case SideN:
			transform_mode_size_off.x = 0;
			transform_mode_size_off.y = glm::min(transform_mode_size_off.y, (int)wnd_size.y - 40);
			break;
		case SideS:
			transform_mode_size_off.x = 0;
			transform_mode_size_off.y = glm::max(transform_mode_size_off.y, -(int)wnd_size.y + 40);
			break;
		case SideE:
			transform_mode_size_off.y = 0;
			transform_mode_size_off.x = glm::max(transform_mode_size_off.x, -(int)wnd_size.x + 40);
			break;
		case SideW:
			transform_mode_size_off.y = 0;
			transform_mode_size_off.x = glm::min(transform_mode_size_off.x, (int)wnd_size.x - 40);
			break;
		case SideNE:
			transform_mode_size_off.x = glm::max(transform_mode_size_off.x, -(int)wnd_size.x + 40);
			transform_mode_size_off.y = glm::min(transform_mode_size_off.y, (int)wnd_size.y - 40);
			break;
		case SideNW:
			transform_mode_size_off.x = glm::min(transform_mode_size_off.x, (int)wnd_size.x - 40);
			transform_mode_size_off.y = glm::min(transform_mode_size_off.y, (int)wnd_size.y - 40);
			break;
		case SideSE:
			transform_mode_size_off.x = glm::max(transform_mode_size_off.x, -(int)wnd_size.x + 40);
			transform_mode_size_off.y = glm::max(transform_mode_size_off.y, -(int)wnd_size.y + 40);
			break;
		case SideSW:
			transform_mode_size_off.x = glm::min(transform_mode_size_off.x, (int)wnd_size.x - 40);
			transform_mode_size_off.y = glm::max(transform_mode_size_off.y, -(int)wnd_size.y + 40);
			break;
		}

		ui->begin_status_window();
		if (transform_mode)
		{
			if (transform_mode_moving)
				ui->text("Moving: (%d, %d) %d, %d", 
					transform_mode_move_off.x, transform_mode_move_off.y,
					s->mouse_x, s->mouse_y);
			else if (transform_mode_sizing)
				ui->text("Sizing: (%d, %d) %d, %d",
					transform_mode_size_off.x, transform_mode_size_off.y,
					s->mouse_x, s->mouse_y);
			else
				ui->text_unformatted("Transform mode, press 'T' or 'Esc' to exit.");
		}
		else
			ui->text_unformatted("Ready.");
		auto status_rect = ui->get_curr_window_rect();
		ui->end_window();

		static vec2 off{0.f, 0.f};
		vec2 bg_pos{0.f, menu_rect.w};
		vec2 bg_size{res.x, res.y - 
			(menu_rect.w - menu_rect.y) - 
			(status_rect.w - status_rect.y)};
		static bool graping_grid = false;
		ui->begin_plain_window("background", bg_pos, bg_size);

		for (auto i = mod((int)off.x, 100); i.y < res.x; i.y += 100, i.x--)
		{
			if (i.y < 0)
				continue;
			ui->add_line_to_window(vec2(i.y, 0.f), vec2(i.y, res.y), vec4(1.f));
			ui->add_text_to_window(vec2(i.y + 4, 0.f), vec4(1.f), "%d", i.x * -100);
		}
		for (auto i = mod((int)off.y, 100); i.y < res.y; i.y += 100, i.x--)
		{
			if (i.y < 0)
				continue;
			ui->add_line_to_window(vec2(0.f, i.y), vec2(res.x, i.y), vec4(1.f));
			ui->add_text_to_window(vec2(4.f, i.y), vec4(1.f), "%d", i.x * -100);
		}

		auto want_sel = !transform_mode;

		if (ui->is_curr_window_hovered())
		{
			if (s->mouse_buttons[2] == (KeyStateJust | KeyStateDown))
				graping_grid = true;
			if (want_sel &&
				s->mouse_buttons[0] == (KeyStateJust | KeyStateDown))
			{
				sel = (Widget*)0xFFFFFFFF;
				want_sel = false;
			}
		}

		if (graping_grid)
		{
			off.x += s->mouse_disp_x;
			off.y += s->mouse_disp_y;
			if ((s->mouse_buttons[2] & KeyStateDown) == 0)
				graping_grid = false;
		}
		ui->end_window();

		ui->push_displayrect(vec4(bg_pos, bg_pos + bg_size));
		ui->begin_window(wnd_name.data, wnd_pos + off + bg_pos, wnd_size,
			UI::WindowNoResize);
		auto wnd_inner_rect = ui->get_curr_window_inner_rect();

		for (auto &w : widgets)
		{
			w->show(ui);
			if (ui->is_last_item_hovered() && want_sel &&
				s->mouse_buttons[0] == (KeyStateJust | KeyStateDown))
			{
				w->ID = ui->get_last_ID();
				sel = w.get();
				want_sel = false;
			}
			w->rect = ui->get_last_item_rect();
			if (sel == w.get())
				sel_rect = w->rect;
		}

		if (ui->is_curr_window_hovered() && want_sel &&
			s->mouse_buttons[0] == (KeyStateJust | KeyStateDown))
		{
			sel = nullptr;
			want_sel = false;
		}
		if (sel == nullptr)
			sel_rect = ui->get_curr_window_rect();

		if (transform_mode)
		{
			if (!transform_mode_moving && !transform_mode_sizing)
			{
				auto just_clicked = s->mouse_buttons[0] == (KeyStateJust | KeyStateDown);
				if (just_clicked)
					transform_mode_anchor = ivec2(s->mouse_x, s->mouse_y);
				auto side = rect_side(vec2(s->mouse_x, s->mouse_y), sel_rect, 4.f);
				switch (side)
				{
				case SideN: case SideS:
					transform_mode_cursor = CursorSizeNS;
					break;
				case SideE: case SideW:
					transform_mode_cursor = CursorSizeWE;
					break;
				case SideNE: case SideSW:
					transform_mode_cursor = CursorSizeNESW;
					break;
				case SideNW: case SideSE:
					transform_mode_cursor = CursorSizeNWSE;
					break;
				case InSide:
					transform_mode_cursor = CursorSizeAll;
					break;
				}
				if (side != OutSide)
				{
					ui->set_cursor(transform_mode_cursor);
					if (just_clicked)
					{
						if (side == InSide)
							transform_mode_moving = true;
						else
						{
							transform_mode_sizing = true;
							transform_mode_sizing_side = side;
						}
					}
				}
			}
			if (transform_mode_moving)
			{
				ui->set_cursor(transform_mode_cursor);
				if ((s->mouse_buttons[0] & KeyStateDown) == 0)
				{
					if (sel == nullptr)
						wnd_pos += transform_mode_move_off;
					transform_mode_moving = false;
				}
			}
			if (transform_mode_sizing)
			{
				ui->set_cursor(transform_mode_cursor);
				if ((s->mouse_buttons[0] & KeyStateDown) == 0)
				{
					if (sel == nullptr)
					{
						switch (transform_mode_sizing_side)
						{
						case SideN:
							wnd_pos.y += transform_mode_size_off.y;
							wnd_size.y -= transform_mode_size_off.y;
							break;
						case SideS:
							wnd_size.y += transform_mode_size_off.y;
							break;
						case SideW:
							wnd_pos.x += transform_mode_size_off.x;
							wnd_size.x -= transform_mode_size_off.x;
							break;
						case SideE:
							wnd_size.x += transform_mode_size_off.x;
							break;
						case SideNE:
							wnd_pos.y += transform_mode_size_off.y;
							wnd_size.y -= transform_mode_size_off.y;
							wnd_size.x += transform_mode_size_off.x;
							break;
						case SideNW:
							wnd_pos.y += transform_mode_size_off.y;
							wnd_size.y -= transform_mode_size_off.y;
							wnd_pos.x += transform_mode_size_off.x;
							wnd_size.x -= transform_mode_size_off.x;
							break;
						case SideSE:
							wnd_size.y += transform_mode_size_off.y;
							wnd_size.x += transform_mode_size_off.x;
							break;
						case SideSW:
							wnd_size.y += transform_mode_size_off.y;
							wnd_pos.x += transform_mode_size_off.x;
							wnd_size.x -= transform_mode_size_off.x;
							break;
						}
					}
					transform_mode_sizing = false;
				}
			}
		}

		ui->end_window();
		ui->pop_displayrect();

		ui->push_overlay_cliprect(vec4(bg_pos, bg_pos + bg_size));
		if (sel != (Widget*)0xFFFFFFFF)
		{
			if (!transform_mode)
				expand_rect(sel_rect, 4.f);
			ui->add_rect_to_overlap(sel_rect, vec4(1.f, 1.f, 0.f, 1.f));
			if (transform_mode)
			{
				if (transform_mode_moving)
				{
					ui->add_rect_to_overlap(sel_rect + vec4(transform_mode_move_off, 
						transform_mode_move_off), vec4(1.f));
					ui->add_line_to_overlap(vec2(transform_mode_anchor), vec2(s->mouse_x,
						s->mouse_y), vec4(1.f));
					if (sel != nullptr)
					{
						ui->add_line_to_overlap(vec2(wnd_inner_rect.x, wnd_inner_rect.y), 
							vec2(wnd_inner_rect.z, wnd_inner_rect.y), 
							vec4(1.f, 0.f, 0.f, 1.f));
					}
				}
				if (transform_mode_sizing)
				{
					auto rect = sel_rect;
					switch (transform_mode_sizing_side)
					{
					case SideN:
						rect.y += transform_mode_size_off.y;
						break;
					case SideS:
						rect.w += transform_mode_size_off.y;
						break;
					case SideW:
						rect.x += transform_mode_size_off.x;
						break;
					case SideE:
						rect.z += transform_mode_size_off.x;
						break;
					case SideNE:
						rect.y += transform_mode_size_off.y;
						rect.z += transform_mode_size_off.x;
						break;
					case SideNW:
						rect.y += transform_mode_size_off.y;
						rect.x += transform_mode_size_off.x;
						break;
					case SideSE:
						rect.w += transform_mode_size_off.y;
						rect.z += transform_mode_size_off.x;
						break;
					case SideSW:
						rect.w += transform_mode_size_off.y;
						rect.x += transform_mode_size_off.x;
						break;
					}
					ui->add_rect_to_overlap(rect, vec4(1.f));
				}
			}
		}
		ui->pop_overlay_cliprect();

		ui->begin_window("Hierarchy", vec2(get_inf()), vec2(get_inf()), UI::WindowSaveSetting);
		for (auto &w : widgets)
		{
			if (ui->selectable(w->name.data, sel == w.get()) &&
				want_sel)
			{
				sel = w.get();
				want_sel = false;
			}
		}
		ui->end_window();

		ui->begin_window("Inspector", vec2(get_inf()), vec2(get_inf()), UI::WindowSaveSetting);
		if (sel != (Widget*)0xFFFFFFFF)
		{
			if (sel == nullptr)
			{
				ui->text("Window:");
				ui->inputtext("name", wnd_name.data, sizeof(wnd_name.data));
				ui->dragfloat2("pos", &wnd_pos, 1.f);
				ui->dragfloat2("size", &wnd_size, 1.f);
			}
			else
			{
				switch (sel->type)
				{
				case WidgetText:
				{
					auto t = (TextWidget*)sel;
					ui->text("Text:");
					ui->inputtext("Name", t->name.data, sizeof(t->name.data));
					ui->inputtext("Text", t->text.data, sizeof(t->text.data));
				}
					break;
				case WidgetButton:
				{
					auto b = (ButtonWidget*)sel;
					ui->text("Button:");
					ui->inputtext("Label", b->name.data, sizeof(b->name.data));
				}
					break;
				}
			}
		}
		ui->end_window();

		ui->end();

		if (!ui->processed_keyboard_input)
		{
			if (s->key_states['T'] == (KeyStateJust | KeyStateDown))
				toggle_transform_mode();
			if (s->key_states[VK_ESCAPE] == (KeyStateJust | KeyStateDown))
			{
				if (transform_mode)
					transform_mode = false;
			}
		}

		auto index = sc->acquire_image(image_avalible);

		cb_ui->begin();
		ui->record_commandbuffer(cb_ui, rp_ui, fbs_ui[index]);
		cb_ui->end();

		d->q->submit(cb_ui, image_avalible, ui_finished);
		d->q->wait_idle();
		d->q->present(index, sc, ui_finished);

		static long long last_fps = 0;
		if (last_fps != sm->fps)
			printf("%lld\n", sm->fps);
		last_fps = sm->fps;
	});

	return 0;
}
