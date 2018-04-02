
#include <Windows.h>

#include <flame/global.h>

namespace flame
{


	void add_keydown_listener(const std::function<void(int)> &e)
	{
		_keydown_listeners.push_back(e);
	}

	void add_keyup_listener(const std::function<void(int)> &e)
	{
		_keyup_listeners.push_back(e);
	}

	void add_char_listener(const std::function<void(int)> &e)
	{
		_char_listeners.push_back(e);
	}

	void remove_keydown_listener(const std::function<void(int)> &e)
	{
		for (auto it = _keydown_listeners.begin(); it != _keydown_listeners.end(); it++)
		{
			if (TK_GET_ADDRESS(*it) == TK_GET_ADDRESS(e))
			{
				_keydown_listeners.erase(it);
				return;
			}
		}
	}

	void remove_keyup_listener(const std::function<void(int)> &e)
	{
		for (auto it = _keyup_listeners.begin(); it != _keyup_listeners.end(); it++)
		{
			if (TK_GET_ADDRESS(*it) == TK_GET_ADDRESS(e))
			{
				_keyup_listeners.erase(it);
				return;
			}
		}
	}

	void remove_char_listener(const std::function<void(int)> &e)
	{
		for (auto it = _char_listeners.begin(); it != _char_listeners.end(); it++)
		{
			if (TK_GET_ADDRESS(*it) == TK_GET_ADDRESS(e))
			{
				_char_listeners.erase(it);
				return;
			}
		}
	}

	void input_on_frame_begin()
	{
		mouse.disp_x = mouse.x - mouse.prev_x;
		mouse.disp_y = mouse.y - mouse.prev_y;
	}

	void input_on_frame_end()
	{
		for (auto i = 0; i < 3; i++)
		{
			mouse.button[i].just_down = false;
			mouse.button[i].just_up = false;
		}
		mouse.prev_x = mouse.x;
		mouse.prev_y = mouse.y;
		mouse.scroll = 0;
		for (int i = 0; i < TK_ARRAYSIZE(key_states); i++)
			key_states[i].reset();
	}
}
