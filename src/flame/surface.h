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

#include <flame/type.h>
#include <flame/math.h>

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

	enum Key
	{
		Key_Unknown,
		Key_Backspace,
		Key_Tab,
		Key_Enter,
		Key_Shift,
		Key_Ctrl,
		Key_Alt,
		Key_Pause,
		Key_CapsLock,
		Key_Esc,
		Key_Space,
		Key_PgUp,
		Key_PgDn,
		Key_End,
		Key_Home,
		Key_Left,
		Key_Up,
		Key_Right,
		Key_Down,
		Key_PrtSc,
		Key_Ins,
		Key_Del,
		Key_0,
		Key_1,
		Key_2,
		Key_3,
		Key_4,
		Key_5,
		Key_6,
		Key_7,
		Key_8,
		Key_9,
		Key_A,
		Key_B,
		Key_C,
		Key_D,
		Key_E,
		Key_F,
		Key_G,
		Key_H,
		Key_I,
		Key_J,
		Key_K,
		Key_L,
		Key_M,
		Key_N,
		Key_O,
		Key_P,
		Key_Q,
		Key_R,
		Key_S,
		Key_T,
		Key_U,
		Key_V,
		Key_W,
		Key_X,
		Key_Y,
		Key_Z,
		Key_Numpad0,
		Key_Numpad1,
		Key_Numpad2,
		Key_Numpad3,
		Key_Numpad4,
		Key_Numpad5,
		Key_Numpad6,
		Key_Numpad7,
		Key_Numpad8,
		Key_Numpad9,
		Key_Add,
		Key_Subtract,
		Key_Multiply,
		Key_Divide,
		Key_Separator,
		Key_Decimal,
		Key_F1,
		Key_F2,
		Key_F3,
		Key_F4,
		Key_F5,
		Key_F6,
		Key_F7,
		Key_F8,
		Key_F9,
		Key_F10,
		Key_F11,
		Key_F12,
		Key_NumLock,
		Key_ScrollLock
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
		Ivec2 pos;
		Ivec2 size;
		int style;
		std::string title;

		int key_states[128];

		Ivec2 mouse_pos;
		Ivec2 mouse_disp;
		int mouse_scroll;

		int mouse_buttons[3]; // left, right, middle of KeyState

		SurfacePrivate *_priv;
		
		// mouse just down
		inline bool just_down_M(int idx)
		{
			return mouse_buttons[idx] == (KeyStateJust | KeyStateDown);
		}

		// mouse just up
		inline bool just_up_M(int idx)
		{
			return mouse_buttons[idx] == (KeyStateJust | KeyStateUp);
		}

		// mouse pressing
		inline bool pressing_M(int idx)
		{
			return (mouse_buttons[idx] & KeyStateDown) != 0;
		}

		// key just down
		inline bool just_down_K(Key k)
		{
			return key_states[k] == (KeyStateJust | KeyStateDown);
		}

		// key just up
		inline bool just_up_K(Key k)
		{
			return key_states[k] == (KeyStateJust | KeyStateUp);
		}

		// key pressing
		inline bool pressing_K(Key k)
		{
			return (key_states[k] & KeyStateDown) != 0;
		}

		FLAME_SURFACE_EXPORTS void *get_win32_handle();
		FLAME_SURFACE_EXPORTS void *get_standard_cursor(CursorType type);
		FLAME_SURFACE_EXPORTS void set_cursor(void *c);
		FLAME_SURFACE_EXPORTS void show_cursor(bool show);

		FLAME_SURFACE_EXPORTS void set_size(const Ivec2 &_pos, const Ivec2 &_size, int _style);
		FLAME_SURFACE_EXPORTS void set_maximized(bool v);

		FLAME_SURFACE_EXPORTS void *add_keydown_listener(const std::function<void(Surface *, int)> &e);
		FLAME_SURFACE_EXPORTS void *add_keyup_listener(const std::function<void(Surface *, int)> &e);
		FLAME_SURFACE_EXPORTS void *add_char_listener(const std::function<void(Surface *, int)> &e);
		FLAME_SURFACE_EXPORTS void *add_mousedown_listener(const std::function<void(Surface *, int, const Ivec2 &pos)> &e);
		FLAME_SURFACE_EXPORTS void *add_mouseup_listener(const std::function<void(Surface *, int, const Ivec2 &pos)> &e);
		FLAME_SURFACE_EXPORTS void *add_mousemove_listener(const std::function<void(Surface *, const Ivec2 &pos)> &e);
		FLAME_SURFACE_EXPORTS void *add_mousescroll_listener(const std::function<void(Surface *, int)> &e);
		FLAME_SURFACE_EXPORTS void *add_resize_listener(const std::function<void(Surface *, const Ivec2 &size)> &e);
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

		// Acceptable keys: Key_Shift, Key_Ctrl and Key_Alt.
		// left_or_right - 0: left, 1: right
		FLAME_SURFACE_EXPORTS bool is_modifier_pressing(Key k, int left_or_right);
	};

	struct SurfaceManagerPrivate;

	struct SurfaceManager
	{
		long long fps;
		float elapsed_time; // second

		SurfaceManagerPrivate *_priv;

		FLAME_SURFACE_EXPORTS Surface *create_surface(const Ivec2 &_size, int _style, const std::string &_title);
		FLAME_SURFACE_EXPORTS void     destroy_surface(Surface *s);
		FLAME_SURFACE_EXPORTS int      run(const std::function<void()> &idle_callback);
	};

	FLAME_SURFACE_EXPORTS SurfaceManager *create_surface_manager();
	FLAME_SURFACE_EXPORTS void            destroy_surface_manager(SurfaceManager *m);
}
