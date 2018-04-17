#include <flame/time.h>
#include <flame/system.h>
#include <flame/surface.h>
#include <flame/image.h>
#include <flame/math.h>
#include <flame/model/model.h>
#include <flame/graphics/device.h>
#include <flame/graphics/swapchain.h>
#include <flame/graphics/renderpass.h>
#include <flame/graphics/pipeline.h>
#include <flame/graphics/descriptor.h>
#include <flame/graphics/buffer.h>
#include <flame/graphics/texture.h>
#include <flame/graphics/sampler.h>
#include <flame/graphics/framebuffer.h>
#include <flame/graphics/commandbuffer.h>
#include <flame/graphics/semaphore.h>
#include <flame/graphics/queue.h>

#include <algorithm>

int main(int argc, char **args)
{
	using namespace flame;
	using namespace graphics;
	using namespace glm;

	auto sm = create_surface_manager();
	auto s = sm->create_surface(1280, 720, SurfaceStyleFrame,
		"Hello");

	auto d = create_device(true, 1280, 720);

	auto sc = create_swapchain(d, s->get_win32_handle(), s->cx, s->cy);

	auto q = create_queue(d);
	auto cp = create_commandpool(d);

	struct UBO
	{
		mat4 proj;
		mat4 view;
		mat4 model;
	};

	auto ub = create_buffer(d, sizeof(mat4) * 3, BufferUsageUniformBuffer, 
		MemPropHost | MemPropHostCoherent);
	ub->map();
	auto ubo = (UBO*)ub->mapped;
	ubo->proj = mat4(
		vec4(1.f, 0.f, 0.f, 0.f),
		vec4(0.f, -1.f, 0.f, 0.f),
		vec4(0.f, 0.f, 1.f, 0.f),
		vec4(0.f, 0.f, 0.f, 1.f)
	) * perspective(radians(d->fovy), d->aspect, d->near_plane, d->far_plane);;
	ubo->view = lookAt(vec3(0.f, 0.f, 10.f), vec3(0.f), vec3(0.f, 1.f, 0.f));
	ubo->model = mat4(1.f);

	Format depth_format;
	depth_format.v = Format::Depth16;

	auto depth_tex = create_texture(d, -1, -1, 1, 1, depth_format, 
		TextureUsageAttachment, MemPropDevice);
	auto depth_tex_view = create_textureview(d, depth_tex);

	auto rp = create_renderpass(d);
	rp->add_attachment_swapchain(sc, true);
	rp->add_attachment(depth_format, true);
	rp->add_subpass({0}, 1);
	rp->build();

	auto p = create_pipeline(d, rp, 0);
	p->set_vertex_attributes({{
			VertexAttributeFloat3, 
			VertexAttributeFloat2,
			VertexAttributeFloat3
	}});
	p->set_size(-1, -1);
	p->set_depth_test(true);
	p->set_depth_write(true);
	p->add_shader("test/test.vert", {});
	p->add_shader("test/test.frag", {});
	p->build();

	auto dp = create_descriptorpool(d);
	auto ds = dp->create_descriptorset(p, 0);
	ds->set_uniformbuffer(0, 0, ub);

	auto m = load_model("../../Vulkan/data/models/voyager/voyager.dae");
	auto mvs = m->get_vertex_semantics();
	auto mvc = m->get_vertex_count();
	auto mic = m->get_indice_count();

	auto vb = create_buffer(d, mvc * m->get_vertex_size(0), BufferUsageVertexBuffer | 
		BufferUsageTransferDst, MemPropDevice);
	auto ib = create_buffer(d, mic * sizeof(int), BufferUsageIndexBuffer |
		BufferUsageTransferDst, MemPropDevice);
	auto sb = create_buffer(d, std::max(vb->size, ib->size), BufferUsageTransferSrc, 
		MemPropHost | MemPropHostCoherent);
	sb->map();
	{
		auto c = cp->create_commandbuffer();
		c->begin(true);
		memcpy(sb->mapped, m->get_vertexes(0), vb->size);
		c->copy_buffer(sb, vb, 0, 0, vb->size);
		c->end();
		q->submit(c, nullptr, nullptr);
		q->wait_idle();
		c->begin(true);
		memcpy(sb->mapped, m->get_indices(), ib->size);
		c->copy_buffer(sb, ib, 0, 0, ib->size);
		c->end();
		q->submit(c, nullptr, nullptr);
		q->wait_idle();
		cp->destroy_commandbuffer(c);
	}
	sb->unmap();

	auto sampler = create_sampler(d, FilterLinear, FilterLinear,
		false);

	auto m_map = create_texture_from_file(d, cp, q, "../../Vulkan/data/models/voyager/voyager_bc3_unorm.ktx");
	auto m_map_view = create_textureview(d, m_map);
	ds->set_texture(1, 0, m_map_view, sampler);

	Framebuffer *fbs[2];
	Commandbuffer *cbs[2];
	for (auto i = 0; i < 2; i++)
	{
		fbs[i] = create_framebuffer(d);
		fbs[i]->set_size(-1, -1);
		fbs[i]->set_renderpass(rp);
		fbs[i]->set_view_swapchain(0, sc, i);
		fbs[i]->set_view(1, depth_tex_view);
		fbs[i]->build();

		cbs[i] = cp->create_commandbuffer();
		cbs[i]->begin();
		cbs[i]->begin_renderpass(rp, fbs[i]);
		cbs[i]->bind_pipeline(p);
		cbs[i]->bind_descriptorset(ds);
		cbs[i]->bind_vertexbuffer(vb);
		cbs[i]->bind_indexbuffer(ib, IndiceTypeUint);
		cbs[i]->draw_indexed(m->get_indice_count(), 0);
		//cbs[i]->draw(m->get_vertex_count());
		cbs[i]->end_renderpass();
		cbs[i]->end();
	}

	auto image_avalible = create_semaphore(d);
	auto render_finished = create_semaphore(d);

	auto x_ang = 0.f;
	auto view_changed = false;
	s->add_mousemove_listener([&](Surface *s, int, int){
		if (s->mouse_buttons[0] & KeyStateDown)
		{
			x_ang += s->mouse_disp_x;
			view_changed = true;
		}
	});

	sm->run([&](){
		if (view_changed)
		{
			ubo->model = rotate(radians(x_ang), vec3(0.f, 1.f, 0.f));

			view_changed = false;
		}

		auto index = sc->acquire_image(image_avalible);
		q->submit(cbs[index], image_avalible, render_finished);
		q->present(index, sc, render_finished);

		//static long long last_ns = 0;
		//auto t = get_now_ns();
		//if (t - last_ns >= 10000000)
		//{
		//	static int B = 0;
		//	vec4 color(0.5f, 0.7f, (B / 255.f), 1.f);
		//	memcpy(ub->mapped, &color, sizeof(vec4));
		//	B++;
		//	if (B == 256)
		//		B = 0;
		//	last_ns = t;
		//}

		static long long last_fps = 0;
		if (last_fps != sm->fps)
			printf("%lld\n", sm->fps);
		last_fps = sm->fps;
	});

	return 0;
}
