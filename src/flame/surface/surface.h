#pragma once

#include <memory>
#include <string>
#include <functional>

#include <flame/exports.h>
#include <flame/global.h>

namespace flame
{
	struct InputState
	{
		bool just_down;
		bool just_up;
		bool pressing;

		InputState() :
			just_down(false),
			just_up(false),
			pressing(false)
		{
		}

		void reset()
		{
			just_down = just_up = false;
		}

		void on_down()
		{
			just_down = true;
			just_up = false;
			pressing = true;
		}

		void on_up()
		{
			just_down = false;
			just_up = true;
			pressing = false;
		}
	};

	FLAME_EXPORTS extern InputState key_states[256];

	struct Mouse
	{
		int x;
		int y;
		int prev_x;
		int prev_y;
		int disp_x;
		int disp_y;
		int scroll;
		InputState button[3]; // left, right, middle
	};

	FLAME_EXPORTS extern Mouse mouse;

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

		int cx;
		int cy;
		int style;
		std::string title;
	};

	FLAME_EXPORTS Surface *create_surface(int _cx, int _cy, int _style, const std::string &_title);
	FLAME_EXPORTS void destroy_surface(Surface *s);

	FLAME_EXPORTS void set_surface_size(Surface *s, int _cx, int _cy, int _style);
	FLAME_EXPORTS void set_surface_maximized(Surface *s, bool v);

	FLAME_EXPORTS void *add_keydown_listener(Surface *s, const std::function<void(Surface *, int)> &e);
	FLAME_EXPORTS void *add_keyup_listener(Surface *s, const std::function<void(Surface *, int)> &e);
	FLAME_EXPORTS void *add_char_listener(Surface *s, const std::function<void(Surface *, int)> &e);
	FLAME_EXPORTS void *add_resize_listener(Surface *s, const std::function<void(Surface *, int, int)> &e);

	FLAME_EXPORTS void remove_keydown_listener(Surface *s, void *p);
	FLAME_EXPORTS void remove_keyup_listener(Surface *s, void *p);
	FLAME_EXPORTS void remove_char_listener(Surface *s, void *p);
	FLAME_EXPORTS void remove_resize_listener(Surface *s, void *p);

	void input_on_frame_begin();
	void input_on_frame_end();

}
