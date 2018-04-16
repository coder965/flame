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
#include <flame/graphics/framebuffer.h>
#include <flame/graphics/commandbuffer.h>
#include <flame/graphics/semaphore.h>
#include <flame/graphics/queue.h>

#include <algorithm>

int main(int argc, char **args)
{
	auto sm = flame::create_surface_manager();
	auto s = sm->create_surface(1280, 720, flame::SurfaceStyleFrame,
		"Hello");

	auto d = flame::graphics::create_device(true, 1280, 720);

	auto sc = flame::graphics::create_swapchain(d, s->get_win32_handle(), s->cx, s->cy);

	auto q = flame::graphics::create_queue(d);
	auto cp = flame::graphics::create_commandpool(d);

	struct UBO
	{
		glm::mat4 proj;
		glm::mat4 view;
		glm::mat4 model;
	}ubo;

	auto ub = flame::graphics::create_buffer(d, sizeof(glm::mat4) * 3, flame::graphics::BufferUsageUniformBuffer, flame::graphics::MemPropHost |
		flame::graphics::MemPropHostCoherent);
	ub->map();
	ubo.proj = glm::mat4(
		glm::vec4(1.f, 0.f, 0.f, 0.f),
		glm::vec4(0.f, -1.f, 0.f, 0.f),
		glm::vec4(0.f, 0.f, 1.f, 0.f),
		glm::vec4(0.f, 0.f, 0.f, 1.f)
	) * glm::perspective(glm::radians(d->fovy), d->aspect, d->near_plane, d->far_plane);;
	ubo.view = glm::lookAt(glm::vec3(0.f, 0.f, 10.f), glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.f));
	ubo.model = glm::mat4(1.f);
	glm::vec4 color(0.5f, 0.7f, 0.f, 1.f);
	memcpy(ub->mapped, &ubo, ub->size);

	auto rp = flame::graphics::create_renderpass(d);
	rp->add_attachment_swapchain(sc, true);
	rp->add_subpass({0}, -1);
	rp->build();

	auto p = flame::graphics::create_pipeline(d, rp, 0);
	p->set_vertex_attributes({{flame::graphics::VertexAttributeFloat3, 
		flame::graphics::VertexAttributeFloat2,
		flame::graphics::VertexAttributeFloat3}});
	p->set_size(-1, -1);
	p->set_cull_mode(flame::graphics::CullModeNone);
	p->add_shader("test/test.vert", {});
	p->add_shader("test/test.frag", {});
	p->build();

	auto dp = flame::graphics::create_descriptorpool(d);
	auto ds = dp->create_descriptorset(p, 0);
	ds->set_uniformbuffer(0, 0, ub);

	auto m = flame::load_model("voyager/voyager.dae");
	auto mvs = m->get_vertex_semantics();
	auto mvc = m->get_vertex_count();
	auto mic = m->get_indice_count();

	auto vb = flame::graphics::create_buffer(d, mvc * m->get_vertex_size(0), flame::graphics::BufferUsageVertexBuffer | 
		flame::graphics::BufferUsageTransferDst, flame::graphics::MemPropDevice);
	auto ib = flame::graphics::create_buffer(d, mic * sizeof(int), flame::graphics::BufferUsageIndexBuffer |
		flame::graphics::BufferUsageTransferDst, flame::graphics::MemPropDevice);
	auto sb = flame::graphics::create_buffer(d, std::max(vb->size, ib->size), flame::graphics::BufferUsageTransferSrc, flame::graphics::MemPropHost |
		flame::graphics::MemPropHostCoherent);
	sb->map();
	{
		auto c = cp->create_commandbuffer();
		c->begin(true);
		memcpy(sb->mapped, m->get_vertexes(0), vb->size);
		flame::graphics::copy_buffer(c, sb, vb, 0, 0, vb->size);
		c->end();
		q->submit(c, nullptr, nullptr);
		q->wait_idle();
		c->begin(true);
		memcpy(sb->mapped, m->get_indices(), ib->size);
		flame::graphics::copy_buffer(c, sb, ib, 0, 0, ib->size);
		c->end();
		q->submit(c, nullptr, nullptr);
		q->wait_idle();
		cp->destroy_commandbuffer(c);
	}
	sb->unmap();

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
		cbs[i]->bind_vertexbuffer(vb);
		cbs[i]->bind_indexbuffer(ib, flame::graphics::IndiceTypeUint);
		cbs[i]->draw_indexed(m->get_indice_count());
		//cbs[i]->draw(m->get_vertex_count());
		cbs[i]->end_renderpass();
		cbs[i]->end();
	}

	auto image_avalible = flame::graphics::create_semaphore(d);
	auto render_finished = flame::graphics::create_semaphore(d);

	sm->run([&](){
		auto index = sc->acquire_image(image_avalible);
		q->submit(cbs[index], image_avalible, render_finished);
		q->present(index, sc, render_finished);

		//static long long last_ns = 0;
		//auto t = flame::get_now_ns();
		//if (t - last_ns >= 10000000)
		//{
		//	static int B = 0;
		//	glm::vec4 color(0.5f, 0.7f, (B / 255.f), 1.f);
		//	memcpy(ub->mapped, &color, sizeof(glm::vec4));
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
