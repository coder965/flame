#include <flame/system.h>
#include <flame/surface.h>
#include <flame/image.h>
#include <flame/graphics/graphics.h>
#include <flame/graphics/swapchain.h>
#include <flame/graphics/renderpass.h>
#include <flame/graphics/framebuffer.h>

int main(int argc, char **args)
{
	auto m = flame::create_surface_manager();
	auto s = m->create_surface(300, 300, flame::SurfaceStyleFrame, 
		"Hello");

	auto g = flame::graphics::create_graphics(false, 1280, 720);

	auto sc = flame::graphics::create_swapchain(g, s->get_win32_handle(), s->cx, s->cy);

	auto rp = flame::graphics::create_renderpass(g);
	rp->add_attachment_swapchain(sc, true);
	rp->add_subpass({0}, -1);
	rp->create();

	flame::graphics::Framebuffer *fbs[2];
	for (auto i = 0; i < 2; i++)
	{
		fbs[i] = flame::graphics::create_framebuffer(g);
		fbs[i]->cx = 1280;
		fbs[i]->cy = 720;
		fbs[i]->set_renderpass(rp);
		fbs[i]->set_view_swapchain(0, sc, i);
		fbs[i]->create();
	}

	m->run([&](){
		static long long last_fps = 0;
		if (last_fps != m->fps)
			printf("%lld\n", m->fps);
		last_fps = m->fps;
	});

	return 0;
}
