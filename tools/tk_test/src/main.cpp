#include <flame/surface.h>

int main(int argc, char **args)
{
	auto m = flame::create_surface_manager();
	auto s = flame::create_surface(m, 400, 300, 
		flame::SurfaceStyleFrame | 
		flame::SurfaceStyleResizable, 
		"Hello");

	flame::surface_manager_run(m, [](){
		static int f = 0;
		static int q = 0;
		f++;
		if (f == 1000000)
		{
			printf("%d\n", q++);
			f = 0;
		}
	});

	return 0;
}
