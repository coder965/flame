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
	graphics::Commandbuffer *cbs_ui[2];
	for (auto i = 0; i < 2; i++)
	{
		fbs_ui[i] = create_framebuffer(d, res.x, res.y, rp_ui);
		fbs_ui[i]->set_view_swapchain(0, sc, i);
		fbs_ui[i]->build();

		cbs_ui[i] = d->cp->create_commandbuffer();
		cbs_ui[i]->begin();
		cbs_ui[i]->end();
	}

	auto image_avalible = graphics::create_semaphore(d);
	auto ui_finished = graphics::create_semaphore(d);

	auto ui = UI::create_instance(d, rp_ui);

	static MediumString title;
	strcpy(title.data, "Window");
	static vec2 wnd_pos{ 0.f, 0.f };
	sm->run([&](){
		ui->begin(res.x, res.y, sm->elapsed_time, s->mouse_x, s->mouse_y,
			(s->mouse_buttons[0] & KeyStateDown) != 0,
			(s->mouse_buttons[1] & KeyStateDown) != 0,
			(s->mouse_buttons[2] & KeyStateDown) != 0,
			s->mouse_scroll);

		static unsigned int sel_ID = -1;
		/*	sel_ID
			0xFFFFFFFF:selecting nothing
			         0:selecting the window
			        >0:selecting an item
		*/
		auto sel_ID_changed = false;

		static vec2 off{0.f, 0.f};
		static bool graping_grid = false;
		ui->begin_plain_window("background", vec2(0.f, 0.f), vec2(res.x, res.y));
		auto off_div = (ivec2)off / 100;
		auto off_mod = (ivec2)off % 100;
		auto i = 0;
		for (auto x = off_mod.x; x < res.x; x += 100, i++)
		{
			ui->add_line_to_window(vec2(x, 0.f), vec2(x, res.y), vec4(1.f));
			ui->add_text_to_window(vec2(x + 4, 0.f), vec4(1.f), "%d", int((i - off_div.x) * 100));
		}
		i = 0;
		for (auto y = off_mod.y; y < res.y; y += 100, i++)
		{
			ui->add_line_to_window(vec2(0.f, y), vec2(res.x, y), vec4(1.f));
			ui->add_text_to_window(vec2(4.f, y), vec4(1.f), "%d", int((i - off_div.y) * 100));
		}
		auto background_clicked = false;
		if (ui->is_curr_window_hovered())
		{
			if (s->mouse_buttons[2] == (KeyStateJust | KeyStateDown))
				graping_grid = true;
			if (s->mouse_buttons[0] == (KeyStateJust | KeyStateDown))
				background_clicked = true;
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

		ui->begin_window(title.data, need_set_wnd_pos ? (wnd_pos + off) : vec2(get_inf()),
			vec2(get_inf()));
		auto wnd_rect = ui->get_curr_window_rect();
		if (!need_set_wnd_pos)
			wnd_pos = vec2(wnd_rect.x, wnd_rect.y) - off;
		need_set_wnd_pos = false;
		auto w_focused = ui->is_curr_window_focused();
		ui->button("Button");
		if (ui->is_last_item_hovered() && 
			s->mouse_buttons[0] == (KeyStateJust | KeyStateDown))
		{
			sel_ID = ui->get_last_ID();
			sel_ID_changed = true;
		}
		glm::vec4 rect;
		if (sel_ID == ui->get_last_ID())
			rect = ui->get_last_item_rect();
		if (!sel_ID_changed)
		{
			if (ui->is_curr_window_hovered() &&
				s->mouse_buttons[0] == (KeyStateJust | KeyStateDown))
				sel_ID = 0;
			else if (background_clicked)
				sel_ID = 0xFFFFFFFF;
		}
		if (sel_ID == 0)
			rect = wnd_rect;
		ui->end_window();
		if (sel_ID != 0xFFFFFFFF)
		{
			expand_rect(rect, 4.f);
			ui->add_rect_to_overlap(rect, vec4(1.f, 1.f, 0.f, 1.f));
		}

		ui->begin_window("Inspector", vec2(get_inf()), vec2(get_inf()), true);
		if (sel_ID == 0)
		{
			ui->inputtext("title", title.data, sizeof(title.data));
			if (ui->dragfloat2("pos", &wnd_pos, 1.f))
				need_set_wnd_pos = true;
		}
		ui->end_window();

		ui->end();

		for (auto i = 0; i < 2; i++)
		{
			cbs_ui[i]->begin();
			ui->record_commandbuffer(cbs_ui[i], rp_ui, fbs_ui[i]);
			cbs_ui[i]->end();
		}

		auto index = sc->acquire_image(image_avalible);

		d->q->submit(cbs_ui[index], image_avalible, ui_finished);
		d->q->wait_idle();
		d->q->present(index, sc, ui_finished);

		static long long last_fps = 0;
		if (last_fps != sm->fps)
			printf("%lld\n", sm->fps);
		last_fps = sm->fps;
	});

	return 0;
}
