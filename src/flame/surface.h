#pragma once

#ifdef _FLAME_SURFACE_EXPORTS
#define FLAME_SURFACE_EXPORTS __declspec(dllexport)
#else
#define FLAME_SURFACE_EXPORTS __declspec(dllimport)
#endif

#include <memory>
#include <string>
#include <functional>

#include <flame/global.h>

namespace flame
{
	enum SurfaceStyle
	{
		SurfaceStyleFrame = 1 << 0,
		SurfaceStyleResizable = 1 << 1,
		SurfaceStyleFullscreen = 1 << 2
	};

	enum KeyState
	{
		KeyStateUp = 1 << 0,
		KeyStateDown = 1 << 1,
		KeyStateJust = 1 << 2,
	};

	struct SurfacePrivate;

	struct Surface
	{
		int x;
		int y;
		int cx;
		int cy;
		int style;
		std::string title;

		int key_states[512];

		int mouse_x;
		int mouse_y;
		int mouse_disp_x;
		int mouse_disp_y;
		int mouse_scroll;

		int mouse_buttons[3];

		SurfacePrivate *_priv;

		FLAME_SURFACE_EXPORTS void *get_win32_handle();

		FLAME_SURFACE_EXPORTS void set_size(int _x, int _y, int _cx, int _cy, int _style);
		FLAME_SURFACE_EXPORTS void set_maximized(bool v);

		FLAME_SURFACE_EXPORTS void *add_keydown_listener(const std::function<void(Surface *, int)> &e);
		FLAME_SURFACE_EXPORTS void *add_keyup_listener(const std::function<void(Surface *, int)> &e);
		FLAME_SURFACE_EXPORTS void *add_char_listener(const std::function<void(Surface *, int)> &e);
		FLAME_SURFACE_EXPORTS void *add_mousedown_listener(const std::function<void(Surface *, int, int, int)> &e);
		FLAME_SURFACE_EXPORTS void *add_mouseup_listener(const std::function<void(Surface *, int, int, int)> &e);
		FLAME_SURFACE_EXPORTS void *add_mousemove_listener(const std::function<void(Surface *, int, int)> &e);
		FLAME_SURFACE_EXPORTS void *add_mousescroll_listener(const std::function<void(Surface *, int)> &e);
		FLAME_SURFACE_EXPORTS void *add_resize_listener(const std::function<void(Surface *, int, int)> &e);
		FLAME_SURFACE_EXPORTS void *add_destroy_listener(const std::function<void(Surface *)> &e);

		FLAME_SURFACE_EXPORTS void remove_keydown_listener(void *p);
		FLAME_SURFACE_EXPORTS void remove_keyup_listener(void *p);
		FLAME_SURFACE_EXPORTS void remove_char_listener(void *p);
		FLAME_SURFACE_EXPORTS void remove_mousedown_listener(void *p);
		FLAME_SURFACE_EXPORTS void remove_mouseup_listener(void *p);
		FLAME_SURFACE_EXPORTS void remove_mousemove_listener(void *p);
		FLAME_SURFACE_EXPORTS void remove_mousescroll_listener(void *p);
		FLAME_SURFACE_EXPORTS void remove_resize_listener(void *p);
		FLAME_SURFACE_EXPORTS void remove_destroy_listener(void *p);
	};

	struct SurfaceManagerPrivate;

	struct SurfaceManager
	{
		long long fps;

		SurfaceManagerPrivate *_priv;

		FLAME_SURFACE_EXPORTS Surface *create_surface(int _cx, int _cy, int _style, const std::string &_title);
		FLAME_SURFACE_EXPORTS void     destroy_surface(Surface *s);
		FLAME_SURFACE_EXPORTS int      run(const std::function<void()> &idle_callback);
	};

	FLAME_SURFACE_EXPORTS SurfaceManager *create_surface_manager();
	FLAME_SURFACE_EXPORTS void            destroy_surface_manager(SurfaceManager *m);
}