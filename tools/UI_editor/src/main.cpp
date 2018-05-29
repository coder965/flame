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
#include <flame/graphics/texture.h>
#include <flame/graphics/commandbuffer.h>
#include <flame/graphics/semaphore.h>
#include <flame/graphics/queue.h>
#include <flame/UI/instance.h>

using namespace flame;

graphics::Device *d;
UI::Instance *ui;
graphics::Texture *rpm_tex;

// track UI textures

struct UITextureTracker
{
	MediumString filename;
	int use_count;

	graphics::Texture *tex;
	graphics::Textureview *tv;
};

UITextureTracker ui_texture_trackers[126]; // total = 128, 
										   // first one is reserved for font
										   // second ont is reserved for replace me

int add_ui_texture(const char *filename)
{
	auto first_spare_ID = -1;
	for (auto i = 0; i < 126; i++)
	{
		if (ui_texture_trackers[i].use_count > 0)
		{
			if (strcmp(ui_texture_trackers[i].filename.data, filename) == 0)
			{
				ui_texture_trackers[i].use_count++;
				return i;
			}
		}
		else if (first_spare_ID == -1)
			first_spare_ID = i;
	}

	if (first_spare_ID == -1)
		return -1;

	auto tex = graphics::create_texture_from_file(d, filename);
	if (tex)
	{
		ui_texture_trackers[first_spare_ID].use_count++;
		ui_texture_trackers[first_spare_ID].tex = tex;
		ui_texture_trackers[first_spare_ID].tv = graphics::create_textureview(d, tex);
		ui->set_texture(first_spare_ID + 2, ui_texture_trackers[first_spare_ID].tv);
		return first_spare_ID + 2;
	}
	return -1;
}

int add_ui_texture(int cx, int cy)
{
	for (auto i = 0; i < 126; i++)
	{
		if (ui_texture_trackers[i].use_count == 0)
		{
			ui_texture_trackers[i].use_count++;
			ui_texture_trackers[i].tex = graphics::create_texture(d, cx, cy, 1, 1, graphics::Format_R8G8B8A8_UNORM,
				graphics::TextureUsageAttachment | graphics::TextureUsageShaderSampled,
				graphics::MemPropDevice);
			ui_texture_trackers[i].tv = graphics::create_textureview(d, ui_texture_trackers[i].tex);
			ui->set_texture(i + 2, ui_texture_trackers[i].tv);
			return i + 2;
		}
	}

	return -1;
}

#define ICON_TEXT u8"\uf001"
#define ICON_BUTTON u8"\uf002"
#define ICON_IMAGE u8"\uf003"

int main(int argc, char **args)
{
	Ivec2 res(1280, 720);

	auto sm = create_surface_manager();
	auto s = sm->create_surface(res, SurfaceStyleFrame, "UI Editor");

	d = graphics::create_device(false);

	auto sc = graphics::create_swapchain(d, s->get_win32_handle(), s->size);

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

	ui = UI::create_instance(d, rp_ui, s);
	ui->add_font("icon_.ttf", 0xf001, 0xf003);
	ui->build();

	// add replace me texture to ui
	rpm_tex = graphics::create_texture_from_file(d, "replaceme.jpg");
	auto rpm_tv = graphics::create_textureview(d, rpm_tex);
	ui->set_texture(1, rpm_tv);

	// init ui texture trackers

	for (auto i = 0; i < 126; i++)
		ui_texture_trackers[i].use_count = 0;

	MediumString wnd_name;
	strcpy(wnd_name.data, "New Form");
	Vec2 wnd_pos(50.f, 20.f);
	Vec2 wnd_size(300.f, 400.f);

	enum WidgetType
	{
		WidgetTypeNull,
		WidgetTypeText,
		WidgetTypeButton,
		WidgetTypeImage,
		WidgetTypeParticleWorld
	};

	struct Widget
	{
		Vec2 pos;
		Vec2 size;

		Rect rect;

		WidgetType type;
		ShortString name;
		unsigned int ID;

		virtual void show(UI::Instance *ui, const Vec2 &off) = 0;
	};

	struct WidgetText : Widget
	{
		MediumString text;

		virtual void show(UI::Instance *ui, const Vec2 &off) override
		{
			ui->set_cursor_pos(off + pos);
			ui->ID_text_unformatted(name.data, text.data);
		}
	};

	struct WidgetButton : Widget
	{
		virtual void show(UI::Instance *ui, const Vec2 &off) override
		{
			ui->set_cursor_pos(off + pos);
			ui->button(name.data);
		}
	};

	struct WidgetImage : Widget
	{
		MediumString filename;

		int img_idx;

		virtual void show(UI::Instance *ui, const Vec2 &off) override
		{
			ui->set_cursor_pos(off + pos);
			ui->image(img_idx, size);
		}
	};

	struct WidgetParticleWorld : Widget
	{
		MediumString shader_filename;
		MediumString blueprint_filename;

		int img_idx;

		virtual void show(UI::Instance *ui, const Vec2 &off) override
		{
			ui->set_cursor_pos(off + pos);
			ui->image(img_idx, size);
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
		Rect sel_rect;

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
					auto w = new WidgetText;
					w->type = WidgetTypeText;
					strcpy(w->name.data, input->data);
					strcpy(w->text.data, input->data);
					w->pos = Vec2(10.f, 30.f);
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
					auto w = new WidgetButton;
					w->type = WidgetTypeButton;
					strcpy(w->name.data, input->data);
					w->pos = Vec2(10.f, 30.f);
					widgets.emplace_back(w);
				}, default_name.c_str());
			}
			if (ui->menuitem("Image"))
			{
				std::string default_name;
				for (auto i = 0; ; i++)
				{
					default_name = "Image" + std::to_string(i);
					if (!find_widget(default_name.c_str()))
						break;
				}
				ui->add_input_dialog("Please Enter The ID Of Image", "ID", [&](MediumString *input) {
					if (input->data[0] == 0)
					{
						ui->add_message_dialog("Add Image", "ID cannot be empty");
						return;
					}
					if (find_widget(input->data))
					{
						ui->add_message_dialog("Add Image", "This ID already exists");
						return;
					}

					auto w = new WidgetImage;
					w->type = WidgetTypeImage;
					strcpy(w->name.data, input->data);
					w->pos = Vec2(10.f, 30.f);
					w->size = Vec2(rpm_tex->cx, rpm_tex->cy);
					w->filename.data[0] = 0;
					w->img_idx = 1;
					widgets.emplace_back(w);
				}, default_name.c_str());
			}
			if (ui->menuitem("Particle World"))
			{
				std::string default_name;
				for (auto i = 0; ; i++)
				{
					default_name = "ParticleWorld" + std::to_string(i);
					if (!find_widget(default_name.c_str()))
						break;
				}

				struct AddParticleWorldDialog
				{
					ShortString ID;
					int cx;
					int cy;
				};

				auto dialog_user_data = ui->add_dialog("Add Particle World", sizeof(AddParticleWorldDialog), [&](UI::Instance *ui, void *user_data, bool &open) {
					auto dialog_data = (AddParticleWorldDialog*)user_data;
					ui->inputtext("ID", dialog_data->ID.data,
						sizeof(dialog_data->ID.data), true);
					ui->dragint("cx", &dialog_data->cx, 1.f, 0, 10000);
					ui->dragint("cy", &dialog_data->cy, 1.f, 0, 10000);
					if (ui->button("OK"))
					{
						if (dialog_data->ID.data[0] != 0)
						{
							if (!find_widget(dialog_data->ID.data))
							{
								auto img_idx = add_ui_texture(dialog_data->cx, dialog_data->cy);
								if (img_idx != -1)
								{
									auto w = new WidgetParticleWorld;
									w->type = WidgetTypeParticleWorld;
									strcpy(w->name.data, dialog_data->ID.data);
									w->pos = Vec2(10.f, 30.f);
									w->size = Vec2(dialog_data->cx, dialog_data->cy);
									w->shader_filename.data[0] = 0;
									w->blueprint_filename.data[0] = 0;
									w->img_idx = img_idx;
									widgets.emplace_back(w);
								}
								else
									ui->add_message_dialog("Add Particle World", "No enough space for ui texture (total 126)");
							}
							else
								ui->add_message_dialog("Add Particle World", "This ID already exists");
						}
						else
							ui->add_message_dialog("Add Particle World", "ID cannot be empty");

						open = false;
					}
					ui->sameline();
					if (ui->button("Cancel"))
						open = false;
				});
				auto dialog_data = (AddParticleWorldDialog*)dialog_user_data;
				strcpy(dialog_data->ID.data, default_name.c_str());
				dialog_data->cx = 200;
				dialog_data->cy = 200;
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

		static bool mode_moving = false;
		static bool mode_sizing = false;
		static CursorType cursor;
		static Ivec2 anchor;
		static Rect::Side sizing_side;

		auto move_off = s->mouse_pos - anchor;
		auto size_off = move_off;
		switch (sizing_side)
		{
		case Rect::SideN:
			size_off.x = 0;
			size_off.y = min(size_off.y, wnd_size.y - 40);
			break;
		case Rect::SideS:
			size_off.x = 0;
			size_off.y = max(size_off.y, -wnd_size.y + 40);
			break;
		case Rect::SideE:
			size_off.y = 0;
			size_off.x = max(size_off.x, -wnd_size.x + 40);
			break;
		case Rect::SideW:
			size_off.y = 0;
			size_off.x = min(size_off.x, wnd_size.x - 40);
			break;
		case Rect::SideNE:
			size_off.x = max(size_off.x, -wnd_size.x + 40);
			size_off.y = min(size_off.y, wnd_size.y - 40);
			break;
		case Rect::SideNW:
			size_off.x = min(size_off.x, wnd_size.x - 40);
			size_off.y = min(size_off.y, wnd_size.y - 40);
			break;
		case Rect::SideSE:
			size_off.x = max(size_off.x, -wnd_size.x + 40);
			size_off.y = max(size_off.y, -wnd_size.y + 40);
			break;
		case Rect::SideSW:
			size_off.x = min(size_off.x, wnd_size.x - 40);
			size_off.y = max(size_off.y, -wnd_size.y + 40);
			break;
		}

		ui->begin_status_window();
		if (mode_moving)
			ui->text("Moving: (%d, %d) %d, %d", move_off.x, move_off.y, s->mouse_pos.x, s->mouse_pos.y);
		else if (mode_sizing)
			ui->text("Sizing: (%d, %d) %d, %d", size_off.x, size_off.y, s->mouse_pos.x, s->mouse_pos.y);
		else
			ui->text_unformatted("Ready.");
		auto status_rect = ui->get_curr_window_rect();
		ui->end_window();

		auto dragging_widget = WidgetTypeNull;

		ui->begin_sidebarL_window(menu_rect.max.y);
		ui->button(ICON_TEXT);
		if (ui->is_last_item_active())
			dragging_widget = WidgetTypeText;
		ui->button(ICON_BUTTON);
		if (ui->is_last_item_active())
			dragging_widget = WidgetTypeButton;
		ui->button(ICON_IMAGE);
		if (ui->is_last_item_active())
			dragging_widget = WidgetTypeImage;
		auto sidebar_rect = ui->get_curr_window_rect();
		ui->end_window();

		static Vec2 off(0.f);
		Vec2 bg_pos(sidebar_rect.max.x, menu_rect.max.y);
		Vec2 bg_size(res.x - sidebar_rect.max.x, res.y -
			menu_rect.max.y - status_rect.height());
		static bool graping_grid = false;
		ui->begin_plain_window("background", bg_pos, bg_size);

		ui->get_curr_window_drawlist().draw_grid(bg_pos, off, Vec2(res));

		auto want_sel = true;

		if (ui->is_curr_window_hovered())
		{
			if (s->just_down_M(2))
				graping_grid = true;
			if (want_sel && s->just_down_M(0))
			{
				sel = (Widget*)0xFFFFFFFF;
				want_sel = false;
			}
		}

		if (graping_grid)
		{
			off.x += s->mouse_disp.x;
			off.y += s->mouse_disp.y;
			if (!s->pressing_M(2))
				graping_grid = false;
		}

		auto dl_bg = ui->get_curr_window_drawlist();

		ui->end_window();

		auto last_global_clip = ui->set_global_cliprect(Rect(bg_pos, bg_pos + bg_size));
		ui->begin_window(wnd_name.data, wnd_pos + off + bg_pos, wnd_size,
			UI::WindowNoResize);
		auto wnd_inner_rect = ui->get_curr_window_inner_rect();
		auto wnd_rect = ui->get_curr_window_rect();

		for (auto &w : widgets)
		{
			w->show(ui, wnd_rect.min);
			if (ui->is_last_item_hovered() && want_sel && s->just_down_M(0))
			{
				w->ID = ui->get_last_ID();
				sel = w.get();
				want_sel = false;
			}
			w->rect = ui->get_last_item_rect();
			if (sel == w.get())
				sel_rect = w->rect;
		}

		if (ui->is_curr_window_hovered() && want_sel && s->just_down_M(0))
		{
			sel = nullptr;
			want_sel = false;
		}
		if (sel == nullptr)
			sel_rect = ui->get_curr_window_rect();

		auto dl_wnd = ui->get_curr_window_drawlist();

		if (dragging_widget == WidgetTypeNull && !mode_moving && !mode_sizing)
		{
			auto just_clicked = s->just_down_M(0);
			if (just_clicked)
				anchor = s->mouse_pos;
			auto side = sel_rect.calc_side(Vec2(s->mouse_pos), 4.f);
			switch (side)
			{
			case Rect::SideN: case Rect::SideS:
				cursor = CursorSizeNS;
				break;
			case Rect::SideE: case Rect::SideW:
				cursor = CursorSizeWE;
				break;
			case Rect::SideNE: case Rect::SideSW:
				cursor = CursorSizeNESW;
				break;
			case Rect::SideNW: case Rect::SideSE:
				cursor = CursorSizeNWSE;
				break;
			case Rect::Inside:
				cursor = CursorSizeAll;
				break;
			}
			if (side != Rect::Outside)
			{
				ui->set_mousecursor(cursor);
				if (just_clicked)
				{
					if (side == Rect::Inside)
						mode_moving = true;
					else
					{
						mode_sizing = true;
						sizing_side = side;
					}
				}
			}
		}
		if (mode_moving)
		{
			ui->set_mousecursor(cursor);
			if (!s->pressing_M(0))
			{
				if (sel == nullptr)
					wnd_pos += move_off;
				else if (sel != (Widget*)0xFFFFFFFF)
					sel->pos += move_off;;
				mode_moving = false;
			}
		}
		if (mode_sizing)
		{
			ui->set_mousecursor(cursor);
			if (!s->pressing_M(0))
			{
				if (sel == nullptr)
				{
					switch (sizing_side)
					{
					case Rect::SideN:
						wnd_pos.y += size_off.y;
						wnd_size.y -= size_off.y;
						break;
					case Rect::SideS:
						wnd_size.y += size_off.y;
						break;
					case Rect::SideW:
						wnd_pos.x += size_off.x;
						wnd_size.x -= size_off.x;
						break;
					case Rect::SideE:
						wnd_size.x += size_off.x;
						break;
					case Rect::SideNE:
						wnd_pos.y += size_off.y;
						wnd_size.y -= size_off.y;
						wnd_size.x += size_off.x;
						break;
					case Rect::SideNW:
						wnd_pos.y += size_off.y;
						wnd_size.y -= size_off.y;
						wnd_pos.x += size_off.x;
						wnd_size.x -= size_off.x;
						break;
					case Rect::SideSE:
						wnd_size.y += size_off.y;
						wnd_size.x += size_off.x;
						break;
					case Rect::SideSW:
						wnd_size.y += size_off.y;
						wnd_pos.x += size_off.x;
						wnd_size.x -= size_off.x;
						break;
					}
				}
				mode_sizing = false;
			}
		}

		ui->end_window();
		ui->set_global_cliprect(last_global_clip);

		auto dl_ol = ui->get_overlap_drawlist();

		if (dragging_widget != WidgetTypeNull)
		{
			dl_ol.add_text(Vec2(s->mouse_pos), Vec4(1.f, 1.f, 0.f, 1.f),
				ICON_IMAGE);
		}
		if (sel != (Widget*)0xFFFFFFFF)
		{
			sel_rect.expand(1.f);
			(sel == nullptr ? dl_bg : dl_wnd).add_rect(sel_rect, Vec4(1.f, 1.f, 0.f, 1.f));
			if (mode_moving)
			{
				dl_ol.add_rect(sel_rect + Vec2(move_off), Vec4(1.f));
				dl_ol.add_line(Vec2(anchor), Vec2(s->mouse_pos), Vec4(1.f));
				//if (sel != nullptr)
				//{
				//	dl_ol.add_line(Vec2(wnd_inner_rect.min.x, wnd_inner_rect.min.y),
				//		Vec2(wnd_inner_rect.max.x, wnd_inner_rect.min.y), 
				//		Vec4(1.f, 0.f, 0.f, 1.f));
				//}
			}
			if (mode_sizing)
			{
				auto rect = sel_rect;
				switch (sizing_side)
				{
				case Rect::SideN:
					rect.min.y += size_off.y;
					break;
				case Rect::SideS:
					rect.max.y += size_off.y;
					break;
				case Rect::SideW:
					rect.min.x += size_off.x;
					break;
				case Rect::SideE:
					rect.max.x += size_off.x;
					break;
				case Rect::SideNE:
					rect.min.y += size_off.y;
					rect.max.x += size_off.x;
					break;
				case Rect::SideNW:
					rect.min.y += size_off.y;
					rect.min.x += size_off.x;
					break;
				case Rect::SideSE:
					rect.max.y += size_off.y;
					rect.max.x += size_off.x;
					break;
				case Rect::SideSW:
					rect.max.y += size_off.y;
					rect.min.x += size_off.x;
					break;
				}
				dl_ol.add_rect(rect, Vec4(1.f));
			}
		}

		ui->begin_window("Hierarchy", Vec2(get_inf()), Vec2(get_inf()), UI::WindowSaveSetting);
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

		ui->begin_window("Inspector", Vec2(get_inf()), Vec2(get_inf()), UI::WindowSaveSetting);
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
				case WidgetTypeText:
				{
					auto t = (WidgetText*)sel;
					ui->text("Text:");
					ui->inputtext("Name", t->name.data, sizeof(t->name.data));
					ui->inputtext("Text", t->text.data, sizeof(t->text.data));
					break;
				}
				case WidgetTypeButton:
				{
					auto b = (WidgetButton*)sel;
					ui->text("Button:");
					ui->inputtext("Label", b->name.data, sizeof(b->name.data));
					break;
				}
				case WidgetTypeImage:
				{
					auto i = (WidgetImage*)sel;
					ui->text("Image:");
					ui->inputtext("Name", i->name.data, sizeof(i->name.data));
					ui->text("Filename:%s", i->filename.data);
					ui->sameline();
					if (ui->button("set"))
					{
						ui->add_input_dialog("Enter The Filename", "File", [&](MediumString *input) {
							if (input->data[0] == 0)
							{
								i->img_idx = 1;
								i->filename.data[0] = 0;
								return;
							}

							auto id = add_ui_texture(input->data);
							if (id == -1)
							{
								ui->add_message_dialog("Error", "File does not exist or no enough space for ui texture (total 126)");
								return;
							}

							i->img_idx = id;
							strcpy(i->filename.data, input->data);
						});
					}
					ui->dragfloat2("size", &i->size, 0.1f, 0.f, 100000.f);
					if (ui->button("set to file size"))
					{
						auto tex = i->img_idx == 1 ? rpm_tex : 
							ui_texture_trackers[i->img_idx - 2].tex;
						i->size = Vec2(tex->cx, tex->cy);
					}
					break;
				}
				case WidgetTypeParticleWorld:
				{
					auto p = (WidgetParticleWorld*)sel;
					ui->text("Particle World:");
					ui->inputtext("Name", p->name.data, sizeof(p->name.data));
					ui->text("Shader Filename:%s", p->shader_filename.data);
					if (ui->button("set##shader"))
					{
					}
					ui->text("Blue Print Filename:%s", p->blueprint_filename.data);
					if (ui->button("set##blueprint"))
					{
					}
					ui->text("%d %d size", (int)p->size.x, (int)p->size.y);
					break;
				}
				}
			}
		}
		ui->end_window();

		ui->end();

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
