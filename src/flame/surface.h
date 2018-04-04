#pragma once

#include <memory>
#include <string>
#include <functional>

#include <flame/exports.h>
#include <flame/global.h>

namespace flame
{
	struct SurfaceManager
	{
		void *impl;
	};

	enum SurfaceStyle
	{
		SurfaceStyleFrame = 1 << 0,
		SurfaceStyleResizable = 1 << 1,
		SurfaceStyleMaximized = 1 << 2,
		SurfaceStyleFullscreen = 1 << 3
	};

	struct Surface
	{
		void *impl;
	};

	enum KeyState
	{
		KeyStateUp = 1 << 0,
		KeyStateDown = 1 << 1,
		KeyStateJust = 1 << 2,
	};

	FLAME_EXPORTS SurfaceManager *create_surface_manager();
	FLAME_EXPORTS void destroy_surface_manager(SurfaceManager *m);
	FLAME_EXPORTS int surface_manager_run(SurfaceManager *m, void(*idle_callback)());

	FLAME_EXPORTS Surface *create_surface(SurfaceManager *m, int _cx, int _cy, int _style, const std::string &_title);
	FLAME_EXPORTS void destroy_surface(SurfaceManager *m, Surface *s);

	FLAME_EXPORTS std::string get_surface_title(Surface *s);
	FLAME_EXPORTS IVEC2 get_surface_size(Surface *s);

	FLAME_EXPORTS void set_surface_size(Surface *s, int _cx, int _cy, int _style);
	FLAME_EXPORTS void set_surface_maximized(Surface *s, bool v);

	FLAME_EXPORTS void *add_keydown_listener(Surface *s, const std::function<void(Surface *, int)> &e);
	FLAME_EXPORTS void *add_keyup_listener(Surface *s, const std::function<void(Surface *, int)> &e);
	FLAME_EXPORTS void *add_char_listener(Surface *s, const std::function<void(Surface *, int)> &e);
	FLAME_EXPORTS void *add_mousedown_listener(Surface *s, const std::function<void(Surface *, int, int, int)> &e);
	FLAME_EXPORTS void *add_mouseup_listener(Surface *s, const std::function<void(Surface *, int, int, int)> &e);
	FLAME_EXPORTS void *add_mousemove_listener(Surface *s, const std::function<void(Surface *, int, int)> &e);
	FLAME_EXPORTS void *add_mousescroll_listener(Surface *s, const std::function<void(Surface *, int)> &e);
	FLAME_EXPORTS void *add_resize_listener(Surface *s, const std::function<void(Surface *, int, int)> &e);
	FLAME_EXPORTS void *add_destroy_listener(Surface *s, const std::function<void(Surface *)> &e);

	FLAME_EXPORTS void remove_keydown_listener(Surface *s, void *p);
	FLAME_EXPORTS void remove_keyup_listener(Surface *s, void *p);
	FLAME_EXPORTS void remove_char_listener(Surface *s, void *p);
	FLAME_EXPORTS void remove_mousedown_listener(Surface *s, void *p);
	FLAME_EXPORTS void remove_mouseup_listener(Surface *s, void *p);
	FLAME_EXPORTS void remove_mousemove_listener(Surface *s, void *p);
	FLAME_EXPORTS void remove_mousescroll_listener(Surface *s, void *p);
	FLAME_EXPORTS void remove_resize_listener(Surface *s, void *p);
	FLAME_EXPORTS void remove_destroy_listener(Surface *s, void *p);

	FLAME_EXPORTS int get_keyboard_state(Surface *s, int key);
	FLAME_EXPORTS int get_mouse_state(Surface *s, int key);
	FLAME_EXPORTS IVEC2 get_mouse_pos(Surface *s);
	FLAME_EXPORTS IVEC2 get_mouse_pos_disp(Surface *s);
	FLAME_EXPORTS int get_mouse_scroll(Surface *s);

}
