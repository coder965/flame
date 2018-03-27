#pragma once

#include <functional>

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

	extern InputState mouse_button[3]; // left, right, middle
	extern InputState key_states[256];

	int mouseX;
	int mouseY;
	int mousePrevX;
	int mousePrevY;
	int mouseDispX;
	int mouseDispY;
	int mouseScroll;

	void add_keydown_listener(const std::function<void(int)> &e);
	void add_keyup_listener(const std::function<void(int)> &e);
	void add_char_listener(const std::function<void(int)> &e);
	void add_resize_listener(const std::function<void(int, int)> &e);
	void add_destroy_listener(const std::function<void()> &e);

	void remove_keydown_listener(const std::function<void(int)> &e);
	void remove_keyup_listener(const std::function<void(int)> &e);
	void remove_char_listener(const std::function<void(int)> &e);
	void remove_resize_listener(const std::function<void(int, int)> &e);
	void remove_destroy_listener(const std::function<void()> &e);
}
