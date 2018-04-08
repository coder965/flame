#include <flame/system.h>
#include <flame/surface.h>
#include <flame/image.h>
#include <flame/graphics/graphics.h>

int main(int argc, char **args)
{
	auto m = flame::create_surface_manager();
	auto s = m->create_surface(300, 300, 0, 
		"Hello");

	auto g = flame::graphics::create_graphics(false, 1280, 720);

	m->run([&](){
		printf("%lld\n", m->fps);
	});

	return 0;
}
