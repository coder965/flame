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

	auto ui = UI::create_instance(d, rp_ui);

	MediumString wnd_title;
	strcpy(wnd_title.data, "New Form");
	vec2 wnd_pos{ 0.f, 0.f };
	vec2 wnd_size{ 100.f, 100.f };

	enum WidgetType
	{
		WidgetText,
		WidgetButton
	};

	struct Widget
	{
		WidgetType type;
		ShortString name;
	};

	std::vector<Widget> widgets;

	sm->run([&](){
		ui->begin(res.x, res.y, sm->elapsed_time, s->mouse_x, s->mouse_y,
			(s->mouse_buttons[0] & KeyStateDown) != 0,
			(s->mouse_buttons[1] & KeyStateDown) != 0,
			(s->mouse_buttons[2] & KeyStateDown) != 0,
			s->mouse_scroll);

		/*
		0xFFFFFFFF:selecting nothing
		         0:selecting the window
		        >0:selecting an item
		*/
		static unsigned int sel_ID = 0xFFFFFFFF;
		glm::vec4 sel_rect;

		static vec2 off{0.f, 0.f};
		static bool graping_grid = false;
		ui->begin_plain_window("background", vec2(0.f, 0.f), vec2(res.x, res.y));

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

		if (ui->is_curr_window_hovered())
		{
			if (s->mouse_buttons[2] == (KeyStateJust | KeyStateDown))
				graping_grid = true;
			if (s->mouse_buttons[0] == (KeyStateJust | KeyStateDown))
				sel_ID = 0xFFFFFFFF;
		}
		static auto need_set_wnd_pos = true;
		if (graping_grid)
		{
			off.x += s->mouse_disp_x;
			off.y += s->mouse_disp_y;
			if ((s->mouse_buttons[2] & KeyStateDown) == 0)
				graping_grid = false;
			need_set_wnd_pos = true;
		}
		ui->end_window();

		ui->begin_window(wnd_title.data, need_set_wnd_pos ? (wnd_pos + off) : vec2(get_inf()),
			vec2(get_inf()));
		auto wnd_rect = ui->get_curr_window_rect();
		if (!need_set_wnd_pos)
			wnd_pos = vec2(wnd_rect.x, wnd_rect.y) - off;
		need_set_wnd_pos = false;

		ui->button("Button");
		if (ui->is_last_item_hovered() && 
			s->mouse_buttons[0] == (KeyStateJust | KeyStateDown))
			sel_ID = ui->get_last_ID();
		if (sel_ID == ui->get_last_ID())
			sel_rect = ui->get_last_item_rect();

		if (ui->is_curr_window_hovered() &&
			s->mouse_buttons[0] == (KeyStateJust | KeyStateDown))
			sel_ID = 0;
		if (sel_ID == 0)
			sel_rect = wnd_rect;
		ui->end_window();

		if (sel_ID != 0xFFFFFFFF)
		{
			expand_rect(sel_rect, 4.f);
			ui->add_rect_to_overlap(sel_rect, vec4(1.f, 1.f, 0.f, 1.f));
		}

		ui->begin_window("Hierarchy", vec2(get_inf()), vec2(get_inf()), true);
		ui->end_window();

		ui->begin_window("Inspector", vec2(get_inf()), vec2(get_inf()), true);
		if (sel_ID == 0)
		{
			ui->text("Window");
			ui->inputtext("title", wnd_title.data, sizeof(wnd_title.data));
			if (ui->dragfloat2("pos", &wnd_pos, 1.f))
				need_set_wnd_pos = true;
			ui->dragfloat2("size", &wnd_size, 1.f);
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
