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

		ui->begin_mainmenu();
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
		auto menu_rect = ui->get_curr_window_rect();
		ui->end_mainmenu();

		ui->begin_status_window();
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

		auto want_sel = true;

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

		auto wnd_rect = ui->get_curr_window_rect();

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
			if (sel == w.get())
				sel_rect = ui->get_last_item_rect();
		}

		if (ui->is_curr_window_hovered() && want_sel &&
			s->mouse_buttons[0] == (KeyStateJust | KeyStateDown))
		{
			sel = nullptr;
			want_sel = false;
		}
		if (sel == nullptr)
			sel_rect = wnd_rect;

		ui->end_window();
		ui->pop_displayrect();

		ui->push_overlay_cliprect(vec4(bg_pos, bg_pos + bg_size));
		if (sel != (Widget*)0xFFFFFFFF)
		{
			expand_rect(sel_rect, 4.f);
			ui->add_rect_to_overlap(sel_rect, vec4(1.f, 1.f, 0.f, 1.f));
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
