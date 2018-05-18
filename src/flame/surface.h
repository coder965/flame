//MIT License
//
//Copyright (c) 2018 wjs
//
//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files (the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions:
//
//The above copyright notice and this permission notice shall be included in all
//copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//SOFTWARE.

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

	enum CursorType
	{
		CursorAppStarting, // arrow and small hourglass
		CursorArrow,
		CursorCross, // unknown
		CursorHand,
		CursorHelp,
		CursorIBeam,
		CursorNo,
		CursorSizeAll,
		CursorSizeNESW,
		CursorSizeNS,
		CursorSizeNWSE,
		CursorSizeWE,
		CursorUpArrwo,
		CursorWait
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

		int mouse_buttons[3]; // left, right, middle of KeyState

		SurfacePrivate *_priv;

		FLAME_SURFACE_EXPORTS void *get_win32_handle();
		FLAME_SURFACE_EXPORTS void *get_standard_cursor(CursorType type);
		FLAME_SURFACE_EXPORTS void set_cursor(void *c);
		FLAME_SURFACE_EXPORTS void show_cursor(bool show);

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
		float elapsed_time; // second

		SurfaceManagerPrivate *_priv;

		FLAME_SURFACE_EXPORTS Surface *create_surface(int _cx, int _cy, int _style, const std::string &_title);
		FLAME_SURFACE_EXPORTS void     destroy_surface(Surface *s);
		FLAME_SURFACE_EXPORTS int      run(const std::function<void()> &idle_callback);
	};

	FLAME_SURFACE_EXPORTS SurfaceManager *create_surface_manager();
	FLAME_SURFACE_EXPORTS void            destroy_surface_manager(SurfaceManager *m);
}
