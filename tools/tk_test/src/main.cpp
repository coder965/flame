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
#include <flame/graphics/framebuffer.h>
#include <flame/graphics/commandbuffer.h>
#include <flame/graphics/semaphore.h>
#include <flame/graphics/queue.h>

int main(int argc, char **args)
{
	auto sm = flame::create_surface_manager();
	auto s = sm->create_surface(1280, 720, flame::SurfaceStyleFrame,
		"Hello");

	auto d = flame::graphics::create_device(true, 1280, 720);

	auto sc = flame::graphics::create_swapchain(d, s->get_win32_handle(), s->cx, s->cy);

	auto q = flame::graphics::create_queue(d);
	auto cp = flame::graphics::create_commandpool(d);

	auto sb = flame::graphics::create_buffer(d, sizeof(glm::vec4), flame::graphics::BufferUsageTransferSrc, flame::graphics::MemPropHost |
		flame::graphics::MemPropHostCoherent);
	sb->map();
	glm::vec4 color(0.5f, 0.7f, 0.f, 1.f);
	memcpy(sb->mapped, &color, sizeof(glm::vec4));
	sb->unmap();

	auto ub = flame::graphics::create_buffer(d, sizeof(glm::vec4), flame::graphics::BufferUsageUniformBuffer |
		flame::graphics::BufferUsageTransferDst, flame::graphics::MemPropDevice);

	{
		auto c = cp->create_commandbuffer();
		c->begin(true);
		flame::graphics::copy_buffer(c, sb, ub, 0, 0, sizeof(glm::vec4));
		c->end();
		q->submit(c, nullptr, nullptr);
		q->wait_idle();
		cp->destroy_commandbuffer(c);
	}

	auto rp = flame::graphics::create_renderpass(d);
	rp->add_attachment_swapchain(sc, true);
	rp->add_subpass({0}, -1);
	rp->build();

	auto p = flame::graphics::create_pipeline(d, rp, 0);
	p->set_size(-1, -1);
	p->set_cull_mode(flame::graphics::CullModeNone);
	p->add_shader("fullscreen.vert", {});
	p->add_shader("test.frag", {});
	p->build();

	auto dp = flame::graphics::create_descriptorpool(d);
	auto ds = dp->create_descriptorset(p, 0);
	ds->set_uniformbuffer(0, 0, ub);

	auto m = flame::load_model("voyager/voyager.dae");
	auto mvs = m->get_vertex_semantics();
	auto mvc = m->get_vertex_count();
	auto mic = m->get_indice_count();

	auto vb = flame::graphics::create_buffer(d, mvc, flame::graphics::BufferUsageVertexBuffer | 
		flame::graphics::BufferUsageTransferDst, flame::graphics::MemPropDevice);
	auto ib = flame::graphics::create_buffer(d, mvc, flame::graphics::BufferUsageIndexBuffer |
		flame::graphics::BufferUsageTransferDst, flame::graphics::MemPropDevice);

	flame::graphics::Framebuffer *fbs[2];
	flame::graphics::Commandbuffer *cbs[2];
	for (auto i = 0; i < 2; i++)
	{
		fbs[i] = flame::graphics::create_framebuffer(d);
		fbs[i]->set_size(-1, -1);
		fbs[i]->set_renderpass(rp);
		fbs[i]->set_view_swapchain(0, sc, i);
		fbs[i]->build();

		cbs[i] = cp->create_commandbuffer();
		cbs[i]->begin();
		cbs[i]->begin_renderpass(rp, fbs[i]);
		cbs[i]->bind_pipeline(p);
		cbs[i]->bind_descriptorset(ds);
		cbs[i]->draw(3);
		cbs[i]->end_renderpass();
		cbs[i]->end();
	}

	auto image_avalible = flame::graphics::create_semaphore(d);
	auto render_finished = flame::graphics::create_semaphore(d);

	sm->run([&](){
		auto index = sc->acquire_image(image_avalible);
		q->submit(cbs[index], image_avalible, render_finished);
		q->present(index, sc, render_finished);

		static long long last_fps = 0;
		if (last_fps != sm->fps)
			printf("%lld\n", sm->fps);
		last_fps = sm->fps;
	});

	return 0;
}
