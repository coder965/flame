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
#include <flame/string.h>
#include <flame/select.h>
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

const float BP_node_titlebar_height = 18.f;

bool running = false;

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
	rp_ui->add_subpass({ 0 }, -1);
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

	// BP = Blue Print

	struct BP_Node;

	struct BP_Slot
	{
		ShortString name;
		Vec2 pos;
		float text_width;

		BP_Node *n;
		int io;

		std::vector<BP_Slot*> links;

		inline bool contains(const Vec2 &p)
		{
			if (p.x > pos.x - 4 && p.x < pos.x + 4 &&
				p.y > pos.y - 4 && p.y < pos.y + 4)
				return true;
			return false;
		}
	};

	enum BP_NodeType
	{
		BP_NodeInterval,
		BP_NodeParticleGenerator
	};

	struct BP_Node
	{
		BP_NodeType type;
		Rect rect;
		std::vector<std::unique_ptr<BP_Slot>> slots[2];

		bool uptodate;

		BP_Node()
		{
			uptodate = false;
		}

		virtual const char*name() = 0;

		virtual void show(UI::Instance *ui, const Vec2 &off) = 0;

		virtual void solve(float elapsed_time) = 0;

		virtual bool get_bool()
		{
			return false;
		}

		inline void add_slot(int io, BP_Slot *s)
		{
			s->n = this;
			s->io = io;
			s->text_width = 0.f;
			slots[io].emplace_back(s);
		}

		inline BP_Slot *hovering_slot(const Vec2 &p)
		{
			for (auto i = 0; i < 2; i++)
			{
				for (auto &s : slots[i])
				{
					if (s->contains(p))
						return s.get();
				}
			}
			return nullptr;
		}
	};

	struct BP_N_Intervaler : BP_Node
	{
		float interval_time;

		float accumulated_time;
		bool signal;

		BP_N_Intervaler()
		{
			type = BP_NodeInterval;
			rect = Rect(0, 0, 200, 60) + Vec2(10);
			auto sl_output = new BP_Slot;
			strcpy(sl_output->name.data, "Output");
			add_slot(1, sl_output);

			interval_time = 1.f;
			accumulated_time = 0.f;
			signal = false;
		}

		virtual const char*name() override
		{
			return "Intervaler";
		}

		virtual void show(UI::Instance *ui, const Vec2 &off) override
		{
			ui->push_item_width(100.f);
			ui->set_cursor_pos(rect.min + Vec2(8.f, BP_node_titlebar_height + 8.f) + off);
			ui->dragfloat("sec", &interval_time, 0.01f, 0.f, 10000.f);
			ui->pop_item_width();
		}

		virtual void solve(float elapsed_time) override
		{
			if (uptodate)
				return;

			signal = false;

			accumulated_time += elapsed_time;
			if (accumulated_time >= interval_time)
			{
				signal = true;
				accumulated_time = 0.f;
			}

			uptodate = true;
		}

		virtual bool get_bool() override
		{
			return signal;
		}
	};

	struct BP_N_ParticleGenerator : BP_Node
	{
		BP_N_ParticleGenerator()
		{
			type = BP_NodeParticleGenerator;
			rect = Rect(0, 0, 150, 60) + Vec2(10);
			auto sl_trigger = new BP_Slot;
			strcpy(sl_trigger->name.data, "Trigger");
			add_slot(0, sl_trigger);
		}

		virtual const char*name() override
		{
			return "Particle Generator";
		}

		virtual void show(UI::Instance *ui, const Vec2 &off) override
		{

		}

		virtual void solve(float elapsed_time) override
		{
			if (uptodate)
				return;

			bool trigger = false;
			for (auto &s : slots[0][0]->links)
			{
				auto n = s->n;
				if (!n->uptodate)
					n->solve(elapsed_time);
				trigger |= n->get_bool();
			}

			if (trigger)
				printf("ParticleGenerator: Triggered\n");

			uptodate = true;
		}
	};

	std::vector<std::unique_ptr<BP_Node>> bp_nodes;

	enum SelType
	{
		SelPtc,
		SelBPn
	};

	Select sel;

	enum TabType
	{
		TabScene,
		TabBluePrint
	};

	auto curr_tab = TabScene;

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
			switch (curr_tab)
			{
			case TabScene:
				if (ui->menuitem("Particle"))
				{
					auto p = new Particle;
					p->coord = Vec2(0.f);
					particles.emplace_back(p);
					need_update_ubo = true;
				}
				break;
			case TabBluePrint:
				if (ui->menuitem("Interval"))
				{
					auto n = new BP_N_Intervaler;
					bp_nodes.emplace_back(n);
				}
				if (ui->menuitem("Particle Generator"))
				{
					auto n = new BP_N_ParticleGenerator;
					bp_nodes.emplace_back(n);
				}
				break;
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
				switch (curr_tab)
				{
				case TabScene:
					if (sel.type == SelPtc)
					{
						for (auto it = particles.begin(); it != particles.end(); it++)
						{
							if (it->get() == (Particle*)sel.v)
							{
								particles.erase(it);
								break;
							}
						}
						sel.reset();
						need_update_ubo = true;
					}
					break;
				case TabBluePrint:
					break;
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

		if (ui->button("Run"))
			running = true;
		ui->sameline();
		if (ui->button("Pause"))
			running = false;
		ui->sameline();
		if (ui->button("Stop"))
		{

		}
		ui->separator();

		ui->begin_tabbar("tabbar");

		if (ui->tabitem("scene"))
		{
			curr_tab = TabScene;

			ui->image(1, effect_size);
			auto img_rect = ui->get_last_item_rect();
			auto dl_ws = ui->get_curr_window_drawlist();

			auto mpos = Vec2(s->mouse_x, s->mouse_y);
			mpos -= effect_size / 2;
			mpos -= Vec2(img_rect.min.x, img_rect.min.y);

			if (ui->is_last_item_hovered() && s->mouse_buttons[0] == (KeyStateJust | KeyStateDown))
			{
				auto clicked_blank = true;
				for (int i = particles.size() - 1; i >= 0; i--)
				{
					Rect r(particles[i]->coord - Vec2(50), particles[i]->coord + Vec2(50));
					if (r.contains(mpos))
					{
						sel.set(SelPtc, particles[i].get());
						clicked_blank = false;
						break;
					}
				}
				if (clicked_blank)
					sel.reset();
			}

			if (sel.type == SelPtc)
			{
				auto sel_ptc = (Particle*)sel.v;
				if (ui->is_last_item_hovered() && (s->mouse_buttons[0] & KeyStateDown) != 0)
				{
					if (s->mouse_disp_x != 0 || s->mouse_disp_y != 0)
					{
						sel_ptc->coord.x += s->mouse_disp_x;
						sel_ptc->coord.y += s->mouse_disp_y;
						update_ubo();
					}
				}

				dl_ws.add_rect(Rect(sel_ptc->coord - Vec2(50), sel_ptc->coord + Vec2(50)) +
					effect_size / 2 + Vec2(img_rect.min.x, img_rect.min.y), Vec4(1, 1, 0, 1));
			}
		}

		if (ui->tabitem("blueprint"))
		{
			curr_tab = TabBluePrint;

			ui->begin_child("bp", Vec2(800, 600), true);

			auto wnd_rect = ui->get_curr_window_rect();

			auto mpos = Vec2(s->mouse_x, s->mouse_y);
			mpos -= wnd_rect.min;

			auto dl = ui->get_curr_window_drawlist();

			const Vec4 n_colors[] = {
				Vec4(0.5f, 0.5f, 1.f, 1.f),
				Vec4(1.f, 0.5f, 0.5f, 1.f)
			};

			static BP_Slot *dragging_slot = nullptr;
			auto node_selected_idx_in_this_frame = -1;

			auto node_num = 0;
			for (auto &n : bp_nodes)
			{
				auto pos = n->rect.min + wnd_rect.min;
				auto size = n->rect.max - n->rect.min;

				ui->push_ID(node_num);

				if (sel.v == n.get())
				{
					dl.add_rect(n->rect.get_expanded(4.f) + wnd_rect.min,
						Vec4(1.f, 1.f, 0.f, 1.f));
				}

				dl.add_rect_filled(Rect(pos, pos + Vec2(size.x, BP_node_titlebar_height)),
					n_colors[n->type], 4.f, true, true, false, false);
				dl.add_rect_filled(Rect(pos + Vec2(0.f, BP_node_titlebar_height), pos + size), 
					Vec4(0.3f, 0.3f, 0.3f, 1.f), 4.f,
					false, false);
				dl.add_text(pos + Vec2(6.f, 2.f), Vec4(1.f), n->name());

				n->show(ui, wnd_rect.min);

				for (auto i = 0; i < 2; i++)
				{
					auto disp = (n->rect.height() - BP_node_titlebar_height) / (n->slots[i].size() + 1);
					auto y = disp;
					for (auto &s : n->slots[i])
					{
						s->pos = Vec2(s->io == 0 ? n->rect.min.x + 8.f : n->rect.max.x - 8.f,
							BP_node_titlebar_height + y + n->rect.min.y);
						dl.add_circle_filled(s->pos + wnd_rect.min,
							4, Vec4(1.f, 1.f, 0.f, 1.f));
						ui->set_cursor_pos(s->pos +
							Vec2(s->io == 1 ?  -8.f : 8.f, -7.f) + wnd_rect.min);
						if (s->io == 1)
							ui->text_unformatted_RA(s->name.data);
						else
							ui->text_unformatted(s->name.data);
						s->text_width = ui->get_last_item_rect().width();

						if (i == 1)
						{
							for (auto ss : s->links)
							{
								auto p1 = s->pos + wnd_rect.min;
								auto p2 = ss->pos + wnd_rect.min;
								
								dl.add_bezier(p1, p1 + Vec2(50.f, 0.f),
									p2 + Vec2(-50.f, 0.f), p2, Vec4(1.f), 3);
							}
						}

						y += disp;
					}
				}

				ui->set_cursor_pos(pos);
				ui->invisibleitem("node", size);

				auto active = ui->is_last_item_active();
				if (active)
				{
					sel.set(SelBPn, n.get());
					node_selected_idx_in_this_frame = node_num;

					auto slot = n->hovering_slot(mpos);
					if (slot)
						dragging_slot = slot;

					if (!dragging_slot && (s->mouse_disp_x != 0 || s->mouse_disp_y != 0))
						n->rect += Vec2(s->mouse_disp_x, s->mouse_disp_y);
				}

				ui->pop_ID();

				node_num++;
			}

			if (s->just_down(0))
			{
				if (node_selected_idx_in_this_frame == -1)
					sel.reset();
				else
				{
					if (node_selected_idx_in_this_frame != bp_nodes.size() - 1)
						std::swap(bp_nodes[node_selected_idx_in_this_frame], bp_nodes[bp_nodes.size() - 1]);
				}
			}

			if (dragging_slot)
			{
				dl.add_circle(dragging_slot->pos + wnd_rect.min,
					6, Vec4(1.f, 1.f, 0.f, 1.f));
				if ((s->mouse_buttons[0] & KeyStateDown) != 0)
				{
					auto p1 = dragging_slot->pos + wnd_rect.min;
					auto p2 = mpos + wnd_rect.min;
					dl.add_bezier(p1, p1 + Vec2(50.f * (dragging_slot->io == 1 ? 1.f : -1.f), 0.f),
						p2, p2, Vec4(1.f), 3);
				}
				else
				{
					for (int i = bp_nodes.size() - 1; i >= 0; i--)
					{
						auto n = bp_nodes[i].get();
						auto s = n->hovering_slot(mpos);
						if (s && dragging_slot != s)
						{
							if (dragging_slot != s)
							{
								auto already_exist = false;
								for (auto ss : dragging_slot->links)
								{
									if (ss == s)
									{
										already_exist = true;
										break;
									}
								}
								if (!already_exist)
								{
									for (auto ss : s->links)
									{
										if (ss == dragging_slot)
										{
											already_exist = true;
											break;
										}
									}

									if (!already_exist)
									{
										dragging_slot->links.push_back(s);
										s->links.push_back(dragging_slot);
									}
								}
							}
						}
					}
					dragging_slot = nullptr;
				}
			}

			ui->end_child();
		}

		ui->end_tabbar();

		ui->end_window();

		ui->end();

		if (running)
		{
			for (auto &n : bp_nodes)
				n->uptodate = false;

			for (auto &n : bp_nodes)
				n->solve(sm->elapsed_time);
		}

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
