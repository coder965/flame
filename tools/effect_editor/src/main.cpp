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
#include <flame/graphics/buffer.h>
#include <flame/graphics/texture.h>
#include <flame/graphics/descriptor.h>
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

	Vec2 res(1280, 720);

	auto sm = create_surface_manager();
	auto s = sm->create_surface(res.x, res.y, SurfaceStyleFrame,
		"Effect Editor");

	auto d = graphics::create_device(true);

	auto sc = graphics::create_swapchain(d, s->get_win32_handle(), s->cx, s->cy);

	Vec2 effect_size(800, 600);

	auto t = graphics::create_texture(d, effect_size.x, effect_size.y, 1, 1,
		graphics::Format_R8G8B8A8_UNORM, 
		graphics::TextureUsageAttachment | graphics::TextureUsageShaderSampled,
		graphics::MemPropDevice);
	auto tv = graphics::create_textureview(d, t);

	auto rp_rtt = graphics::create_renderpass(d);
	rp_rtt->add_attachment(t->format, true);
	rp_rtt->add_subpass({ 0 }, -1);
	rp_rtt->build();

	auto fb_rtt = graphics::create_framebuffer(d, effect_size.x, effect_size.y, rp_rtt);
	fb_rtt->set_view(0, tv);
	fb_rtt->build();

	auto particle_liquid_vert = graphics::create_shader(d, "fullscreen.vert");
	particle_liquid_vert->add_define("USE_UV");
	particle_liquid_vert->build();
	auto particle_liquid_frag = graphics::create_shader(d, "effect/particle_liquid.frag");
	particle_liquid_frag->build();

	auto pl_particle_liquid = graphics::create_pipeline(d);
	pl_particle_liquid->set_size(effect_size.x, effect_size.y);
	pl_particle_liquid->set_cull_mode(graphics::CullModeNone);
	pl_particle_liquid->add_shader(particle_liquid_vert);
	pl_particle_liquid->add_shader(particle_liquid_frag);
	pl_particle_liquid->set_renderpass(rp_rtt, 0);
	pl_particle_liquid->build_graphics();

	struct Particle_glsl
	{
		Vec4 coord;
	};

	struct UBO_Particle
	{
		Ivec4 data;
		Particle_glsl particles[256];
	};

	auto ub_particle_liquid = graphics::create_buffer(d, sizeof(UBO_Particle),
		graphics::BufferUsageUniformBuffer,
		graphics::MemPropHost | graphics::MemPropHostCoherent);
	ub_particle_liquid->map();
	auto ubo_particle_liquid = (UBO_Particle*)ub_particle_liquid->mapped;
	ubo_particle_liquid->data.x = 0;
	ubo_particle_liquid->data.y = effect_size.x;
	ubo_particle_liquid->data.z = effect_size.y;

	auto ds_particle_liquid = d->dp->create_descriptorset(pl_particle_liquid, 0);
	ds_particle_liquid->set_uniformbuffer(0, 0, ub_particle_liquid);

	auto cb_effect = d->cp->create_commandbuffer();
	cb_effect->begin();
	cb_effect->begin_renderpass(rp_rtt, fb_rtt);
	cb_effect->bind_pipeline(pl_particle_liquid);
	cb_effect->bind_descriptorset(ds_particle_liquid);
	cb_effect->draw(3, 1, 0);
	cb_effect->end_renderpass();
	cb_effect->end();

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
	auto effect_finished = graphics::create_semaphore(d);
	auto ui_finished = graphics::create_semaphore(d);

	auto ui = UI::create_instance(d, rp_ui, s);
	ui->set_texture(1, tv);

	struct Particle
	{
		Vec2 coord;
	};

	std::vector<std::unique_ptr<Particle>> particles;

	auto update_ubo = [&]() {
		ubo_particle_liquid->data.x = particles.size();

		for (auto i = 0; i < particles.size(); i++)
		{
			ubo_particle_liquid->particles[i].coord = 
				Vec4(particles[i]->coord + effect_size / 2, 0.f, 0.f);
		}
	};

	auto need_update_ubo = true;

	Particle *sel = nullptr;

	sm->run([&](){
		ui->begin(res.x, res.y, sm->elapsed_time);

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
			if (ui->menuitem("Particle"))
			{
				auto p = new Particle;
				p->coord = Vec2(0.f);
				particles.emplace_back(p);
				need_update_ubo = true;
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
			{
				if (sel)
				{
					for (auto it = particles.begin(); it != particles.end(); it++)
					{
						if (it->get() == sel)
						{
							particles.erase(it);
							break;
						}
					}
					sel = nullptr;
					need_update_ubo = true;
				}
			}
			ui->end_menu();
		}
		if (ui->begin_menu("View"))
		{
			ui->end_menu();
		}
		auto menu_rect = ui->get_curr_window_rect();
		ui->end_mainmenu();

		ui->begin_status_window();
		ui->text_unformatted("Ready.");
		auto status_rect = ui->get_curr_window_rect();
		ui->end_window();

		Vec2 ws_pos(0.f, menu_rect.max.y);
		Vec2 ws_size(res.x, res.y -
			(menu_rect.max.y - menu_rect.min.y) -
			(status_rect.max.y - status_rect.min.y));
		ui->begin_plain_window("ws", ws_pos, ws_size);

		ui->begin_tabbar("tabbar");

		if (ui->tabitem("scene"))
		{
			ui->image(1, effect_size);
			auto img_rect = ui->get_last_item_rect();
			auto dl_ws = ui->get_curr_window_drawlist();

			static Vec2 anchor;

			if (ui->is_curr_window_hovered() && s->mouse_buttons[0] == (KeyStateJust | KeyStateDown))
			{
				auto clicked_blank = true;
				auto mpos = Vec2(s->mouse_x, s->mouse_y);
				mpos -= effect_size / 2;
				mpos -= Vec2(img_rect.min.x, img_rect.min.y);
				for (int i = particles.size() - 1; i >= 0; i--)
				{
					Rect r(particles[i]->coord - Vec2(50), particles[i]->coord + Vec2(50));
					if (r.contains(mpos))
					{
						sel = particles[i].get();
						anchor = mpos;
						clicked_blank = false;
						break;
					}
				}
				if (clicked_blank)
					sel = nullptr;
			}

			if (sel != nullptr)
			{
				if ((s->mouse_buttons[0] & KeyStateDown) != 0)
				{
					if (s->mouse_disp_x != 0 || s->mouse_disp_y != 0)
					{
						sel->coord.x += s->mouse_disp_x;
						sel->coord.y += s->mouse_disp_y;
						update_ubo();
					}
				}

				dl_ws.add_rect(Rect(sel->coord - Vec2(50), sel->coord + Vec2(50)) +
					effect_size / 2 + Vec2(img_rect.min.x, img_rect.min.y), Vec4(1, 1, 0, 1));
			}
		}

		if (ui->tabitem("blueprint"))
		{

		}

		ui->end_tabbar();

		ui->end_window();

		ui->end();

		if (need_update_ubo)
		{
			update_ubo();
			need_update_ubo = false;
		}

		auto index = sc->acquire_image(image_avalible);

		cb_ui->begin();
		ui->record_commandbuffer(cb_ui, rp_ui, fbs_ui[index]);
		cb_ui->end();

		d->q->submit(cb_effect, image_avalible, effect_finished);
		d->q->submit(cb_ui, effect_finished, ui_finished);
		d->q->wait_idle();
		d->q->present(index, sc, ui_finished);

		static long long last_fps = 0;
		if (last_fps != sm->fps)
			printf("%lld\n", sm->fps);
		last_fps = sm->fps;
	});

	return 0;
}
