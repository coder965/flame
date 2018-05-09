#include <flame/time.h>
#include <flame/system.h>
#include <flame/surface.h>
#include <flame/image.h>
#include <flame/math.h>
#include <flame/model/model.h>
#include <flame/graphics/device.h>
#include <flame/graphics/swapchain.h>
#include <flame/graphics/renderpass.h>
#include <flame/graphics/shader.h>
#include <flame/graphics/pipeline.h>
#include <flame/graphics/descriptor.h>
#include <flame/graphics/buffer.h>
#include <flame/graphics/texture.h>
#include <flame/graphics/sampler.h>
#include <flame/graphics/framebuffer.h>
#include <flame/graphics/commandbuffer.h>
#include <flame/graphics/semaphore.h>
#include <flame/graphics/queue.h>
#include <flame/physics/device.h>
#include <flame/physics/material.h>
#include <flame/physics/scene.h>
#include <flame/physics/rigid.h>
#include <flame/physics/shape.h>
#include <flame/UI/UI.h>

#include <algorithm>

int main(int argc, char **args)
{
	using namespace flame;
	using namespace glm;

	auto near_plane = 0.1f;
	auto far_plane = 1000.f;
	auto fovy = 60.f;

	vec2 res(1280, 720);

	auto aspect = (float)res.x / res.y;

	auto sm = create_surface_manager();
	auto s = sm->create_surface(res.x, res.y, SurfaceStyleFrame,
		"Hello");

	auto d = graphics::create_device(true);

	auto sc = graphics::create_swapchain(d, s->get_win32_handle(), s->cx, s->cy);

	auto q = graphics::create_queue(d);
	auto cp = graphics::create_commandpool(d);

	struct UBO_matrix
	{
		mat4 proj;
		mat4 view;
	};

	auto ub_matrix = graphics::create_buffer(d, sizeof(UBO_matrix), graphics::BufferUsageTransferDst |
		graphics::BufferUsageUniformBuffer, graphics::MemPropDevice);

	struct UBO_matrix_ins
	{
		mat4 model[65536];
	};

	auto ub_matrix_ins = graphics::create_buffer(d, sizeof(UBO_matrix_ins), graphics::BufferUsageTransferDst |
		graphics::BufferUsageStorageBuffer, graphics::MemPropDevice);

	auto ub_stag = graphics::create_buffer(d, ub_matrix->size + ub_matrix_ins->size, graphics::BufferUsageTransferSrc,
		graphics::MemPropHost | graphics::MemPropHostCoherent);
	ub_stag->map();

	struct CopyBufferUpdate
	{
		graphics::Buffer *src;
		graphics::Buffer *dst;
		std::vector<graphics::BufferCopy> ranges;
	};

	std::vector<CopyBufferUpdate> updates;

	auto ubo_matrix = (UBO_matrix*)ub_stag->mapped;
	ubo_matrix->proj = mat4(
		vec4(1.f, 0.f, 0.f, 0.f),
		vec4(0.f, -1.f, 0.f, 0.f),
		vec4(0.f, 0.f, 1.f, 0.f),
		vec4(0.f, 0.f, 0.f, 1.f)
	) * perspective(radians(fovy), aspect, near_plane, far_plane);
	ubo_matrix->view = lookAt(vec3(0.f, 0.f, 10.f), vec3(0.f, 0.f, 0.f), vec3(0.f, 1.f, 0.f));

	CopyBufferUpdate upd;
	upd.src = ub_stag;
	upd.dst = ub_matrix;
	upd.ranges.push_back({0, 0, ub_matrix->size});
	updates.push_back(upd);

	auto ubo_matrix_ins = (UBO_matrix_ins*)((unsigned char*)ub_stag->mapped + ub_matrix->size);

	ModelDescription desc;
	desc.set_to_default();
	auto m = create_cube_model(&desc, 0.5f);
	m->root_node->calc_global_matrix();

	auto depth_tex = graphics::create_texture(d, res.x, res.y, 1, 1, graphics::Format::Depth16,
		graphics::TextureUsageAttachment, graphics::MemPropDevice);
	auto depth_tex_view = graphics::create_textureview(d, depth_tex);

	auto rp = graphics::create_renderpass(d);
	rp->add_attachment_swapchain(sc, true);
	rp->add_attachment(graphics::Format::Depth16, true);
	rp->add_subpass({0}, 1);
	rp->build();

	auto test_vert = graphics::create_shader(d, "test/test.vert");
	test_vert->build();
	auto test_frag = graphics::create_shader(d, "test/test.frag");
	test_frag->build();

	auto p = graphics::create_pipeline(d);
	p->set_renderpass(rp, 0);
	p->set_vertex_attributes({{
			graphics::VertexAttributeFloat3,
			//graphics::VertexAttributeFloat2,
			graphics::VertexAttributeFloat3
	}});
	p->set_size(res.x, res.y);
	p->set_depth_test(true);
	p->set_depth_write(true);
	p->add_shader(test_vert);
	p->add_shader(test_frag);
	p->build_graphics();

	auto dp = graphics::create_descriptorpool(d);
	auto ds = dp->create_descriptorset(p, 0);
	ds->set_uniformbuffer(0, 0, ub_matrix);
	ds->set_storagebuffer(1, 0, ub_matrix_ins);

	auto vb = graphics::create_buffer(d, m->vertex_count * m->vertex_buffers[0].size * sizeof(float), graphics::BufferUsageVertexBuffer |
		graphics::BufferUsageTransferDst, graphics::MemPropDevice);
	auto ib = create_buffer(d, m->indice_count * sizeof(int), graphics::BufferUsageIndexBuffer |
		graphics::BufferUsageTransferDst, graphics::MemPropDevice);
	auto sb = graphics::create_buffer(d, vb->size + ib->size, graphics::BufferUsageTransferSrc,
		graphics::MemPropHost | graphics::MemPropHostCoherent);
	sb->map();
	{
		auto c = cp->create_commandbuffer();
		c->begin(true);
		memcpy(sb->mapped, m->vertex_buffers[0].pVertex, vb->size);
		graphics::BufferCopy r1 = {0, 0, vb->size};
		c->copy_buffer(sb, vb, 1, &r1);
		memcpy((unsigned char*)sb->mapped + vb->size, m->pIndices, ib->size);
		graphics::BufferCopy r2 = {vb->size, 0, ib->size};
		c->copy_buffer(sb, ib, 1, &r2);
		c->end();
		q->submit(c, nullptr, nullptr);
		q->wait_idle();
		cp->destroy_commandbuffer(c);
	}
	sb->unmap();
	destroy_buffer(d, sb);

	auto sampler = graphics::create_sampler(d, graphics::FilterLinear, graphics::FilterLinear,
		false);

	//auto m_map = create_texture_from_file(d, cp, q, "../../Vulkan/data/models/voyager/voyager_bc3_unorm.ktx");
	//auto m_map_view = create_textureview(d, m_map);
	//ds->set_texture(1, 0, m_map_view, sampler);

	graphics::Framebuffer *fbs[2];
	graphics::Commandbuffer *cbs[2];
	graphics::Commandbuffer *cbs_ui[2];
	for (auto i = 0; i < 2; i++)
	{
		fbs[i] = create_framebuffer(d, res.x, res.y, rp);
		fbs[i]->set_view_swapchain(0, sc, i);
		fbs[i]->set_view(1, depth_tex_view);
		fbs[i]->build();

		cbs[i] = cp->create_commandbuffer();
		cbs[i]->begin();
		cbs[i]->begin_renderpass(rp, fbs[i]);
		cbs[i]->bind_pipeline(p);
		cbs[i]->bind_descriptorset(ds);
		cbs[i]->bind_vertexbuffer(vb);
		cbs[i]->bind_indexbuffer(ib, graphics::IndiceTypeUint);
		for (auto j = 0; j < m->mesh_count; j++)
			cbs[i]->draw_indexed(m->meshes[j]->indice_count, m->meshes[j]->indice_base, 0, 1, j);
		cbs[i]->end_renderpass();
		cbs[i]->end();

		cbs_ui[i] = cp->create_commandbuffer();
	}

	auto cb_update = cp->create_commandbuffer();

	auto image_avalible = graphics::create_semaphore(d);
	auto render_finished = graphics::create_semaphore(d);
	auto ui_finished = graphics::create_semaphore(d);

	auto x_ang = 0.f;
	auto matrix_need_update = true;
	s->add_mousemove_listener([&](Surface *s, int, int){
		if (s->mouse_buttons[0] & KeyStateDown)
		{
			x_ang += s->mouse_disp_x;
			matrix_need_update = true;
		}
	});

	auto p_d = physics::create_device();
	auto material = physics::create_material(p_d, 0.5f, 0.5f, 0.6f);
	auto scene = physics::create_scene(p_d, -0.98f, 1);
	auto ball_r = physics::create_dynamic_rigid(p_d, vec3(0.f));
	auto ball_s = physics::create_box_shape(ball_r, material, vec3(0.f), 0.5f, 0.5f, 0.5f);
	scene->add_rigid(ball_r);

	vec3 coord = vec3(0.f);
	vec4 quat = vec4(0.f, 0.f, 0.f, 1.f);

	auto ui = UI::create_instance(d, rp, dp, cp, q);

	sm->run([&](){
		ui->begin(res.x, res.y, sm->elapsed_time, s->mouse_x, s->mouse_y,
			(s->mouse_buttons[0] & KeyStateDown) != 0,
			(s->mouse_buttons[1] & KeyStateDown) != 0,
			(s->mouse_buttons[2] & KeyStateDown) != 0,
			s->mouse_scroll);
		bool need_record_ui = false;
		ui->button("Hello");
		ui->end(&need_record_ui);

		if (need_record_ui)
		{
			for (auto i = 0; i < 2; i++)
			{
				cbs_ui[i]->begin();
				ui->record_commandbuffer(cbs_ui[i], rp, fbs[i]);
				cbs_ui[i]->end();
			}
		}

		static long long last_ns = 0;
		auto t = get_now_ns();
		if (t - last_ns >= 41666666)
		{
			scene->update(1.f / 24);
			ball_r->get_pose(coord, quat);

			last_ns = t;

			matrix_need_update = true;
		}

		if (matrix_need_update)
		{
			for (auto i = 0; i < m->mesh_count; i++)
			{
				ubo_matrix_ins->model[i] = translate(coord) *
					rotate(radians(x_ang), vec3(0.f, 1.f, 0.f));
				if (m->meshes[i]->pNode)
					ubo_matrix_ins->model[i] = ubo_matrix_ins->model[i] * m->meshes[i]->pNode->global_matrix;
			}

			CopyBufferUpdate upd;
			upd.src = ub_stag;
			upd.dst = ub_matrix_ins;
			upd.ranges.push_back({ub_matrix->size, 0, int(m->mesh_count * sizeof(mat4))});
			updates.push_back(upd);

			matrix_need_update = false;
		}

		if (!updates.empty())
		{
			cb_update->begin(true);
			for (auto &u : updates)
				cb_update->copy_buffer(u.src, u.dst, u.ranges.size(), u.ranges.data());
			cb_update->end();
			q->submit(cb_update, nullptr, nullptr);
			q->wait_idle();
			updates.clear();
		}

		auto index = sc->acquire_image(image_avalible);

		q->submit(cbs[index], image_avalible, render_finished);
		q->wait_idle();
		q->submit(cbs_ui[index], render_finished, ui_finished);
		q->wait_idle();
		q->present(index, sc, ui_finished);

		static long long last_fps = 0;
		if (last_fps != sm->fps)
			printf("%lld\n", sm->fps);
		last_fps = sm->fps;
	});

	return 0;
}
