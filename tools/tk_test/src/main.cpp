#include <flame/surface.h>

int main(int argc, char **args)
{
	auto m = flame::create_surface_manager();
	auto s0 = flame::create_surface(m, 400, 300, 
		flame::SurfaceStyleFrame | 
		flame::SurfaceStyleResizable, 
		"Hello");
	auto s1 = flame::create_surface(m, 400, 100,
		flame::SurfaceStyleFrame |
		flame::SurfaceStyleResizable,
		"World");
	flame::surface_manager_run(m, [](){});

	return 0;
}
