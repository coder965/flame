#include <flame/system.h>
#include <flame/surface.h>
#include <flame/image.h>
#include <flame/graphics/device.h>
#include <flame/graphics/swapchain.h>
#include <flame/graphics/renderpass.h>
#include <flame/graphics/framebuffer.h>
#include <flame/graphics/commandbuffer.h>
#include <flame/graphics/semaphore.h>
#include <flame/graphics/queue.h>

int main(int argc, char **args)
{
	auto m = flame::create_surface_manager();
	auto s = m->create_surface(1280, 720, flame::SurfaceStyleFrame,
		"Hello");

	auto d = flame::graphics::create_device(false, 1280, 720);

	auto sc = flame::graphics::create_swapchain(d, s->get_win32_handle(), s->cx, s->cy);

	auto rp = flame::graphics::create_renderpass(d);
	rp->add_attachment_swapchain(sc, true);
	rp->add_subpass({0}, -1);
	rp->create();

	flame::graphics::Framebuffer *fbs[2];
	flame::graphics::Commandbuffer *cbs[2];
	auto cp = flame::graphics::create_commandpool(d);
	for (auto i = 0; i < 2; i++)
	{
		fbs[i] = flame::graphics::create_framebuffer(d);
		fbs[i]->cx = 1280;
		fbs[i]->cy = 720;
		fbs[i]->set_renderpass(rp);
		fbs[i]->set_view_swapchain(0, sc, i);
		fbs[i]->create();
		cbs[i] = cp->create_commandbuffer();
		cbs[i]->begin();
		cbs[i]->begin_renderpass(rp, fbs[i]);
		cbs[i]->end_renderpass();
		cbs[i]->end();
	}

	auto image_avalible = flame::graphics::create_semaphore(d);
	auto render_finished = flame::graphics::create_semaphore(d);

	auto q = flame::graphics::create_queue(d);

	m->run([&](){
		auto index = sc->acquire_image(image_avalible);
		q->submit(cbs[index], image_avalible, render_finished);
		q->present(index, sc, render_finished);

		static long long last_fps = 0;
		if (last_fps != m->fps)
			printf("%lld\n", m->fps);
		last_fps = m->fps;
	});

	return 0;
}
