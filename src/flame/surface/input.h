#pragma once

#include <functional>

#include <flame/global.h>

namespace flame
{
	struct InputState
	{
		bool just_down = false;
		bool just_up = false;
		bool pressing = false;

		InputState();
		void reset();
		void on_down();
		void on_up();
	};

	extern InputState key_states[256];

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

	extern Mouse mouse;

	void add_keydown_listener(const std::function<void(int)> &e);
	void add_keyup_listener(const std::function<void(int)> &e);
	void add_char_listener(const std::function<void(int)> &e);

	void remove_keydown_listener(const std::function<void(int)> &e);
	void remove_keyup_listener(const std::function<void(int)> &e);
	void remove_char_listener(const std::function<void(int)> &e);

	void input_on_frame_begin();
	void input_on_frame_end();
	void handle_input_message_win32(void *hwnd, int msg, TK_ULONG_PTR wParam, TK_LONG_PTR lParam);
}
