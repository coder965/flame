#include <flame/surface.h>

int main(int argc, char **args)
{
	using namespace flame;

	auto sm = create_surface_manager();
	auto s = sm->create_surface(1280, 720, SurfaceStyleFrame,
		"Hello");

	s->add_mousemove_listener([&](Surface *s, int, int){
		if (s->mouse_buttons[0] & KeyStateDown)
		{
		}
	});

	sm->run([&](){
	});

	return 0;
}