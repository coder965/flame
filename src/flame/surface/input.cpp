#include <list>
#include <Windows.h>

#include <flame/global.h>
#include <flame/engine/core/input.h>

namespace flame
{
	InputState::InputState()
	{
		reset();
	}

	void InputState::reset()
	{
		just_down = false;
		just_up = false;
	}

	void InputState::on_down()
	{
		just_down = true;
		just_up = false;
		pressing = true;
	}

	void InputState::on_up()
	{
		just_down = false;
		just_up = true;
		pressing = false;
	}

	InputState key_states[256];

	Mouse mouse;

	static std::list<std::function<void(int)>> _keydown_listeners;
	static std::list<std::function<void(int)>> _keyup_listeners;
	static std::list<std::function<void(int)>> _char_listeners;

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

	void handle_input_message_win32(void *hwnd, int msg, TK_ULONG_PTR wParam, TK_LONG_PTR lParam)
	{
		switch (msg)
		{
			case WM_LBUTTONDOWN:
				mouse.button[0].on_down();
				mouse.x = LOWORD(lParam);
				mouse.y = HIWORD(lParam);
				SetCapture((HWND)hwnd);
				break;
			case WM_LBUTTONUP:
				mouse.button[0].on_up();
				mouse.x = LOWORD(lParam);
				mouse.y = HIWORD(lParam);
				ReleaseCapture();
				break;
			case WM_MBUTTONDOWN:
				mouse.button[2].on_down();
				mouse.x = LOWORD(lParam);
				mouse.y = HIWORD(lParam);
				SetCapture((HWND)hwnd);
				break;
			case WM_MBUTTONUP:
				mouse.button[2].on_up();
				mouse.x = LOWORD(lParam);
				mouse.y = HIWORD(lParam);
				ReleaseCapture();
				break;
			case WM_RBUTTONDOWN:
				mouse.button[1].on_down();
				mouse.x = LOWORD(lParam);
				mouse.y = HIWORD(lParam);
				SetCapture((HWND)hwnd);
				break;
			case WM_RBUTTONUP:
				mouse.button[1].on_up();
				mouse.x = LOWORD(lParam);
				mouse.y = HIWORD(lParam);
				ReleaseCapture();
				break;
			case WM_MOUSEMOVE:
				mouse.x = LOWORD(lParam);
				mouse.y = HIWORD(lParam);
				break;
			case WM_MOUSEWHEEL:
				mouse.scroll += (short)HIWORD(wParam);
				break;
			case WM_KEYDOWN:
				key_states[wParam].on_down();
				for (auto &e : _keydown_listeners)
					e(wParam);
				break;
			case WM_KEYUP:
				key_states[wParam].on_up();
				for (auto &e : _keyup_listeners)
					e(wParam);
				break;
			case WM_CHAR:
				for (auto &e : _char_listeners)
					e(wParam);
				break;
			case WM_DESTROY:
				PostQuitMessage(0);
				break;
		}
	}
}
