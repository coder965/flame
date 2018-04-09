#include <flame/system.h>
#include <flame/surface.h>
#include <flame/image.h>
#include <flame/graphics/graphics.h>

int main(int argc, char **args)
{
	auto m = flame::create_surface_manager();
	auto s = m->create_surface(300, 300, flame::SurfaceStyleFrame, 
		"Hello");

	auto g = flame::graphics::create_graphics(false, 1280, 720);

	auto sc = g->create_swapchain(s->get_win32_handle(), s->cx, s->cy);

	m->run([&](){
		static long long last_fps = 0;
		if (last_fps != m->fps)
			printf("%lld\n", m->fps);
		last_fps = m->fps;
	});

	return 0;
}
