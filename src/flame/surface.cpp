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

#include <algorithm>
#include <list>
#include <queue>
#include <assert.h>
#define NOMINMAX
#include <Windows.h>

#include <flame/time.h>
#include <flame/filesystem.h>
#include <flame/system.h>
#include <flame/image.h>
#include <flame/surface.h>

namespace flame
{
	inline Key Z(int code)
	{
		switch (code)
		{
		case VK_BACK:
			return Key_Backspace;
		case VK_TAB:
			return Key_Tab;
		case VK_RETURN:
			return Key_Enter;
		case VK_SHIFT:
			return Key_Shift;
		case VK_CONTROL:
			return Key_Ctrl;
		case VK_MENU:
			return Key_Alt;
		case VK_PAUSE:
			return Key_Pause;
		case VK_CAPITAL:
			return Key_CapsLock;
		case VK_ESCAPE:
			return Key_Esc;
		case VK_SPACE:
			return Key_Space;
		case VK_PRIOR:
			return Key_PgUp;
		case VK_NEXT:
			return Key_PgDn;
		case VK_END:
			return Key_End;
		case VK_HOME:
			return Key_Home;
		case VK_LEFT:
			return Key_Left;
		case VK_UP:
			return Key_Up;
		case VK_RIGHT:
			return Key_Right;
		case VK_DOWN:
			return Key_Down;
		case VK_SNAPSHOT:
			return Key_PrtSc;
		case VK_INSERT:
			return Key_Ins;
		case VK_DELETE:
			return Key_Del;
		case '0':
			return Key_0;
		case '1':
			return Key_1;
		case '2':
			return Key_2;
		case '3':
			return Key_3;
		case '4':
			return Key_4;
		case '5':
			return Key_5;
		case '6':
			return Key_6;
		case '7':
			return Key_7;
		case '8':
			return Key_8;
		case '9':
			return Key_9;
		case 'A':
			return Key_A;
		case 'B':
			return Key_B;
		case 'C':
			return Key_C;
		case 'D':
			return Key_D;
		case 'E':
			return Key_E;
		case 'F':
			return Key_F;
		case 'G':
			return Key_G;
		case 'H':
			return Key_H;
		case 'I':
			return Key_I;
		case 'J':
			return Key_J;
		case 'K':
			return Key_K;
		case 'L':
			return Key_L;
		case 'M':
			return Key_M;
		case 'N':
			return Key_N;
		case 'O':
			return Key_O;
		case 'P':
			return Key_P;
		case 'Q':
			return Key_Q;
		case 'R':
			return Key_R;
		case 'S':
			return Key_S;
		case 'T':
			return Key_T;
		case 'U':
			return Key_U;
		case 'V':
			return Key_V;
		case 'W':
			return Key_W;
		case 'X':
			return Key_X;
		case 'Y':
			return Key_Y;
		case 'Z':
			return Key_Z;
		case VK_NUMPAD0:
			return Key_Numpad0;
		case VK_NUMPAD1:
			return Key_Numpad1;
		case VK_NUMPAD2:
			return Key_Numpad2;
		case VK_NUMPAD3:
			return Key_Numpad3;
		case VK_NUMPAD4:
			return Key_Numpad4;
		case VK_NUMPAD5:
			return Key_Numpad5;
		case VK_NUMPAD6:
			return Key_Numpad6;
		case VK_NUMPAD7:
			return Key_Numpad7;
		case VK_NUMPAD8:
			return Key_Numpad8;
		case VK_NUMPAD9:
			return Key_Numpad9;
		case VK_ADD:
			return Key_Add;
		case VK_SUBTRACT:
			return Key_Subtract;
		case VK_MULTIPLY:
			return Key_Multiply;
		case VK_DIVIDE:
			return Key_Divide;
		case VK_SEPARATOR:
			return Key_Separator;
		case VK_DECIMAL:
			return Key_Decimal;
		case VK_F1:
			return Key_F1;
		case VK_F2:
			return Key_F2;
		case VK_F3:
			return Key_F3;
		case VK_F4:
			return Key_F4;
		case VK_F5:
			return Key_F5;
		case VK_F6:
			return Key_F6;
		case VK_F7:
			return Key_F7;
		case VK_F8:
			return Key_F8;
		case VK_F9:
			return Key_F9;
		case VK_F10:
			return Key_F10;
		case VK_F11:
			return Key_F11;
		case VK_F12:
			return Key_F12;
		case VK_NUMLOCK:
			return Key_NumLock;
		case VK_SCROLL:
			return Key_ScrollLock;
		default:
			return Key_Unknown;
		}
	}

	enum KeyEventType
	{
		KeyEventNull,
		KeyEventDown,
		KeyEventUp
	};

	struct SurfaceManagerPrivate
	{
		std::list<Surface*> surfaces;

		long long last_time;
		long long last_frame_time;
		long long counting_frame;
	};

	struct SurfacePrivate
	{
		HWND hWnd;

		Ivec2 mouse_prev_pos;

		std::list<std::function<void(Surface *, int)>>					 keydown_listeners;
		std::list<std::function<void(Surface *, int)>>					 keyup_listeners;
		std::list<std::function<void(Surface *, int)>>					 char_listeners;
		std::list<std::function<void(Surface *, int, const Ivec2 &pos)>> mousedown_listeners;
		std::list<std::function<void(Surface *, int, const Ivec2 &pos)>> mouseup_listeners;
		std::list<std::function<void(Surface *, const Ivec2 &pos)>>      mousemove_listeners;
		std::list<std::function<void(Surface *, int)>>					 mousescroll_listeners;
		std::list<std::function<void(Surface *, const Ivec2 &size)>>	 resize_listeners;
		std::list<std::function<void(Surface *)>>						 destroy_listeners;

		KeyEventType key_event_type;
		int key_event_key;

		std::queue<int> char_events;

		KeyEventType mouse_event_type;
		int mouse_event_key;
		bool mouse_move_event;
		int mouse_scroll_event;

		bool resize_event;

		bool destroy_event;
	};

	void *Surface::get_win32_handle()
	{
		return _priv->hWnd;
	}

	void *Surface::get_standard_cursor(CursorType type)
	{
		const char *name;
		switch (type)
		{
		case CursorAppStarting:
			name = IDC_APPSTARTING;
			break;
		case CursorArrow:
			name = IDC_ARROW;
			break;
		case CursorCross:
			name = IDC_CROSS;
			break;
		case CursorHand:
			name = IDC_HAND;
			break;
		case CursorHelp:
			name = IDC_HELP;
			break;
		case CursorIBeam:
			name = IDC_IBEAM;
			break;
		case CursorNo:
			name = IDC_NO;
			break;
		case CursorSizeAll:
			name = IDC_SIZEALL;
			break;
		case CursorSizeNESW:
			name = IDC_SIZENESW;
			break;
		case CursorSizeNS:
			name = IDC_SIZENS;
			break;
		case CursorSizeNWSE:
			name = IDC_SIZENWSE;
			break;
		case CursorSizeWE:
			name = IDC_SIZEWE;
			break;
		case CursorUpArrwo:
			name = IDC_UPARROW;
			break;
		case CursorWait:
			name = IDC_WAIT;
			break;
		}
		return LoadCursor(nullptr, name);
	}

	void Surface::set_cursor(void *c)
	{
		SetCursor((HCURSOR)c);
	}

	void Surface::show_cursor(bool show)
	{
		ShowCursor(show);
	}

	void Surface::set_size(const Ivec2 &_pos, const Ivec2 &_size, int _style)
	{
		if (_size.x > 0)
			size.x = _size.x;
		if (_size.y > 0)
			size.y = _size.y;

		bool style_changed = false;
		if (_style != -1 && _style != style)
		{
			style = _style;
			style_changed = true;
		}

		assert(!(style & SurfaceStyleFullscreen) || (!(style & SurfaceStyleFrame) && !(style & SurfaceStyleResizable)));

		int total_size_x;
		int total_size_y;

		auto win32_style = WS_VISIBLE;
		if (style == 0)
			win32_style |= WS_POPUP | WS_BORDER;
		else
		{
			if (style & SurfaceStyleFullscreen)
			{
				total_size_x = get_screen_cx();
				total_size_y = get_screen_cy();
			}
			if (style & SurfaceStyleFrame)
				win32_style |= WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
			if (style & SurfaceStyleResizable)
				win32_style |= WS_THICKFRAME | WS_MAXIMIZEBOX;
		}

		RECT rect = {0, 0, size.x, size.y};
		AdjustWindowRect(&rect, win32_style, false);
		total_size_x = rect.right - rect.left;
		total_size_y = rect.bottom - rect.top;

		pos.x = _pos.x == -1 ? (get_screen_cx() - total_size_x) / 2 : _pos.x;
		pos.y = _pos.y == -1 ? (get_screen_cy() - total_size_y) / 2 : _pos.y;

		if (_priv->hWnd)
		{
			if (style_changed)
				SetWindowLong(_priv->hWnd, GWL_STYLE, win32_style);
			MoveWindow(_priv->hWnd, pos.x, pos.y, size.x, size.y, true);
		}
		else
		{
			_priv->hWnd = CreateWindowA("tke_wnd", title.c_str(), win32_style,
				pos.x, pos.y, total_size_x, total_size_y, NULL, NULL, (HINSTANCE)get_hinst(), NULL);
		}
	}

	void Surface::set_maximized(bool v)
	{
		ShowWindow(_priv->hWnd, v ? SW_SHOWMAXIMIZED : SW_SHOWNORMAL);
	}

	void *Surface::add_keydown_listener(const std::function<void(Surface *s, int)> &e)
	{
		_priv->keydown_listeners.push_back(e);
		return &_priv->keydown_listeners.back();
	}

	void *Surface::add_keyup_listener(const std::function<void(Surface *s, int)> &e)
	{
		_priv->keyup_listeners.push_back(e);
		return &_priv->keyup_listeners.back();
	}

	void *Surface::add_char_listener(const std::function<void(Surface *s, int)> &e)
	{
		_priv->char_listeners.push_back(e);
		return &_priv->char_listeners.back();
	}

	void *Surface::add_mousedown_listener(const std::function<void(Surface *, int, const Ivec2 &pos)> &e)
	{
		_priv->mousedown_listeners.push_back(e);
		return &_priv->mousedown_listeners.back();
	}

	void *Surface::add_mouseup_listener(const std::function<void(Surface *, int, const Ivec2 &pos)> &e)
	{
		_priv->mouseup_listeners.push_back(e);
		return &_priv->mouseup_listeners.back();
	}

	void *Surface::add_mousemove_listener(const std::function<void(Surface *, const Ivec2 &pos)> &e)
	{
		_priv->mousemove_listeners.push_back(e);
		return &_priv->mousemove_listeners.back();
	}

	void *Surface::add_mousescroll_listener(const std::function<void(Surface *, int)> &e)
	{
		_priv->mousescroll_listeners.push_back(e);
		return &_priv->mousescroll_listeners.back();
	}

	void *Surface::add_resize_listener(const std::function<void(Surface *s, const Ivec2 &size)> &e)
	{
		_priv->resize_listeners.push_back(e);
		return &_priv->resize_listeners.back();
	}

	void *Surface::add_destroy_listener(const std::function<void(Surface *s)> &e)
	{
		_priv->destroy_listeners.push_back(e);
		return &_priv->destroy_listeners.back();
	}

	void Surface::remove_keydown_listener(void *p)
	{
		for (auto it = _priv->keydown_listeners.begin(); it != _priv->keydown_listeners.end(); it++)
		{
			if (&(*it) == p)
			{
				_priv->keydown_listeners.erase(it);
				return;
			}
		}
	}

	void Surface::remove_keyup_listener(void *p)
	{
		for (auto it = _priv->keyup_listeners.begin(); it != _priv->keyup_listeners.end(); it++)
		{
			if (&(*it) == p)
			{
				_priv->keyup_listeners.erase(it);
				return;
			}
		}
	}

	void Surface::remove_char_listener(void *p)
	{
		for (auto it = _priv->char_listeners.begin(); it != _priv->char_listeners.end(); it++)
		{
			if (&(*it) == p)
			{
				_priv->char_listeners.erase(it);
				return;
			}
		}
	}

	void Surface::remove_mousedown_listener(void *p)
	{
		for (auto it = _priv->mousedown_listeners.begin(); it != _priv->mousedown_listeners.end(); it++)
		{
			if (&(*it) == p)
			{
				_priv->mousedown_listeners.erase(it);
				return;
			}
		}
	}

	void Surface::remove_mouseup_listener(void *p)
	{
		for (auto it = _priv->mouseup_listeners.begin(); it != _priv->mouseup_listeners.end(); it++)
		{
			if (&(*it) == p)
			{
				_priv->mouseup_listeners.erase(it);
				return;
			}
		}
	}

	void Surface::remove_mousemove_listener(void *p)
	{
		for (auto it = _priv->mousemove_listeners.begin(); it != _priv->mousemove_listeners.end(); it++)
		{
			if (&(*it) == p)
			{
				_priv->mousemove_listeners.erase(it);
				return;
			}
		}
	}

	void Surface::remove_mousescroll_listener(void *p)
	{
		for (auto it = _priv->mousescroll_listeners.begin(); it != _priv->mousescroll_listeners.end(); it++)
		{
			if (&(*it) == p)
			{
				_priv->mousescroll_listeners.erase(it);
				return;
			}
		}
	}

	void Surface::remove_resize_listener(void *p)
	{
		for (auto it = _priv->keydown_listeners.begin(); it != _priv->keydown_listeners.end(); it++)
		{
			if (&(*it) == p)
			{
				_priv->keydown_listeners.erase(it);
				return;
			}
		}
	}

	void Surface::remove_destroy_listener(void *p)
	{
		for (auto it = _priv->keydown_listeners.begin(); it != _priv->keydown_listeners.end(); it++)
		{
			if (&(*it) == p)
			{
				_priv->keydown_listeners.erase(it);
				return;
			}
		}
	}

	bool Surface::is_modifier_pressing(Key k, int left_or_right)
	{
		return false;
	}

	static LRESULT CALLBACK _wnd_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		auto s = (Surface*)GetWindowLongPtr(hWnd, 0);

		if (s)
		{
			switch (message)
			{
				case WM_KEYDOWN:
					s->key_states[Z(wParam)] = KeyStateDown | KeyStateJust;
					s->_priv->key_event_type = KeyEventDown;
					s->_priv->key_event_key = wParam;
					break;
				case WM_KEYUP:
					s->key_states[Z(wParam)] = KeyStateUp | KeyStateJust;
					s->_priv->key_event_type = KeyEventUp;
					s->_priv->key_event_key = wParam;
					break;
				case WM_CHAR:
					s->_priv->char_events.push(wParam);
					break;
				case WM_LBUTTONDOWN:
					s->mouse_buttons[0] = KeyStateDown | KeyStateJust;
					s->mouse_pos = Ivec2(LOWORD(lParam), HIWORD(lParam));
					s->_priv->mouse_event_type = KeyEventDown;
					s->_priv->mouse_event_key = 0;
					break;
				case WM_LBUTTONUP:
					s->mouse_buttons[0] = KeyStateUp | KeyStateJust;
					s->mouse_pos = Ivec2(LOWORD(lParam), HIWORD(lParam));
					s->_priv->mouse_event_type = KeyEventUp;
					s->_priv->mouse_event_key = 0;
					break;
				case WM_MBUTTONDOWN:
					s->mouse_buttons[2] = KeyStateDown | KeyStateJust;
					s->mouse_pos = Ivec2(LOWORD(lParam), HIWORD(lParam));
					s->_priv->mouse_event_type = KeyEventDown;
					s->_priv->mouse_event_key = 2;
					break;
				case WM_MBUTTONUP:
					s->mouse_buttons[2] = KeyStateUp | KeyStateJust;
					s->mouse_pos = Ivec2(LOWORD(lParam), HIWORD(lParam));
					s->_priv->mouse_event_type = KeyEventUp;
					s->_priv->mouse_event_key = 2;
					break;
				case WM_RBUTTONDOWN:
					s->mouse_buttons[1] = KeyStateDown | KeyStateJust;
					s->mouse_pos = Ivec2(LOWORD(lParam), HIWORD(lParam));
					s->_priv->mouse_event_type = KeyEventDown;
					s->_priv->mouse_event_key = 1;
					break;
				case WM_RBUTTONUP:
					s->mouse_buttons[1] = KeyStateUp | KeyStateJust;
					s->mouse_pos = Ivec2(LOWORD(lParam), HIWORD(lParam));
					s->_priv->mouse_event_type = KeyEventUp;
					s->_priv->mouse_event_key = 1;
					break;
				case WM_MOUSEMOVE:
					s->mouse_pos = Ivec2(LOWORD(lParam), HIWORD(lParam));
					s->_priv->mouse_move_event = true;
					break;
				case WM_MOUSEWHEEL:
					s->mouse_scroll = (short)HIWORD(wParam) > 0 ? 1 : -1;
					s->_priv->mouse_scroll_event = true;
					break;
				case WM_DESTROY:
					s->_priv->destroy_event = true;
				case WM_SIZE:
				{
					auto x = std::max((int)LOWORD(lParam), 1);
					auto y = std::max((int)HIWORD(lParam), 1);
					if (x != s->size.x || y != s->size.y)
					{
						s->size.x = x;
						s->size.y = y;
						s->_priv->resize_event = true;
					}
					break;
				}
			}
		}

		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	Surface *SurfaceManager::create_surface(const Ivec2 &_size, int _style, const std::string &_title)
	{
		static bool initialized = false;
		if (!initialized)
		{
			WNDCLASSEXA wcex;
			wcex.cbSize = sizeof(WNDCLASSEXA);
			wcex.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
			wcex.lpfnWndProc = _wnd_proc;
			wcex.cbClsExtra = 0;
			wcex.cbWndExtra = sizeof(FLAME_ULONG_PTR);
			wcex.hInstance = (HINSTANCE)get_hinst();
			if (std::filesystem::exists("ico.png"))
			{
				auto icon_image = load_image("ico.png");
				icon_image->swap_RB();
				wcex.hIcon = CreateIcon(wcex.hInstance, icon_image->cx, icon_image->cy, 1,
					icon_image->bpp, nullptr, icon_image->data);
				release_image(icon_image);
			}
			else
				wcex.hIcon = 0;
			wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
			wcex.hbrBackground = 0;
			wcex.lpszMenuName = 0;
			wcex.lpszClassName = "tke_wnd";
			wcex.hIconSm = wcex.hIcon;
			RegisterClassExA(&wcex);

			initialized = true;
		}

		auto s = new Surface;
		s->title = _title;

		for (auto i = 0; i < FLAME_ARRAYSIZE(s->key_states); i++)
			s->key_states[i] = KeyStateUp;

		s->mouse_pos = Ivec2(0);
		s->mouse_disp = Ivec2(0);
		s->mouse_scroll = 0;

		for (auto i = 0; i < FLAME_ARRAYSIZE(s->mouse_buttons); i++)
			s->mouse_buttons[i] = KeyStateUp;

		s->_priv = new SurfacePrivate;

		s->_priv->hWnd = 0;

		s->_priv->key_event_type = KeyEventNull;
		s->_priv->key_event_key = 0;

		s->_priv->mouse_prev_pos = Ivec2(0);
		s->_priv->mouse_event_type = KeyEventNull;
		s->_priv->mouse_event_key = 0;
		s->_priv->mouse_move_event = false;
		s->_priv->mouse_scroll_event = false;

		s->_priv->resize_event = false;

		s->_priv->destroy_event = false;

		s->style = 0;
		s->set_size(Ivec2(-1), _size, _style);

		SetWindowLongPtr(s->_priv->hWnd, 0, (LONG_PTR)s);

		_priv->surfaces.push_back(s);

		return s;
	}

	void SurfaceManager::destroy_surface(Surface *s)
	{
		for (auto it = _priv->surfaces.begin(); it != _priv->surfaces.end(); it++)
		{
			if (*it == s)
			{
				_priv->surfaces.erase(it);
				break;
			}
		}

		DestroyWindow(s->_priv->hWnd);
		for (auto &e : s->_priv->destroy_listeners)
			e(s);
		delete s->_priv;
		delete s;
	}

	int SurfaceManager::run(const std::function<void()> &idle_callback)
	{
		if (_priv->surfaces.size() == 0)
			return 1;

		assert(idle_callback);

		_priv->last_time = get_now_ns();
		_priv->last_frame_time = _priv->last_time;
		_priv->counting_frame = _priv->last_time;

		for (;;)
		{
			MSG msg;
			while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}

			for (auto it = _priv->surfaces.begin(); it != _priv->surfaces.end(); )
			{
				auto s = *it;

				s->mouse_disp = s->mouse_pos - s->_priv->mouse_prev_pos;

				if (s->_priv->destroy_event)
				{
					for (auto &e : s->_priv->destroy_listeners)
						e(s);
					delete s->_priv;
					delete s;
					it = _priv->surfaces.erase(it);
				}
				else
				{
					switch (s->_priv->key_event_type)
					{
						case KeyEventDown:
							for (auto &e : s->_priv->keydown_listeners)
								e(s, s->_priv->key_event_key);
							break;
						case KeyEventUp:
							for (auto &e : s->_priv->keyup_listeners)
								e(s, s->_priv->key_event_key);
							break;
					}
					s->_priv->key_event_type = KeyEventNull;

					switch (s->_priv->mouse_event_type)
					{
						case KeyEventDown:
							for (auto &e : s->_priv->mousedown_listeners)
								e(s, s->_priv->mouse_event_key, s->mouse_pos);
							break;
						case KeyEventUp:
							for (auto &e : s->_priv->mouseup_listeners)
								e(s, s->_priv->mouse_event_key, s->mouse_pos);
							break;
					}
					s->_priv->mouse_event_type = KeyEventNull;

					if (!s->_priv->char_events.empty())
					{
						while (!s->_priv->char_events.empty())
						{
							auto c = s->_priv->char_events.front();
							for (auto &e : s->_priv->char_listeners)
								e(s, c);
							s->_priv->char_events.pop();
						}
					}

					if (s->_priv->mouse_move_event)
					{
						for (auto &e : s->_priv->mousemove_listeners)
							e(s, s->mouse_pos);
						s->_priv->mouse_move_event = false;
					}

					if (s->_priv->mouse_scroll_event)
					{
						for (auto &e : s->_priv->mousescroll_listeners)
							e(s, s->mouse_scroll);
						s->_priv->mouse_scroll_event = false;
					}

					if (s->_priv->resize_event)
					{
						for (auto &e : s->_priv->resize_listeners)
							e(s, s->size);
						s->_priv->resize_event = false;
					}

					it++;
				}
			}

			if (_priv->surfaces.empty())
				return 0;
			
			if (_priv->last_time - _priv->last_frame_time >= 1000000000)
			{
				fps = _priv->counting_frame;
				_priv->counting_frame = 0;
				_priv->last_frame_time = _priv->last_time;
			}

			idle_callback();

			for (auto s : _priv->surfaces)
			{
				for (int i = 0; i < FLAME_ARRAYSIZE(s->key_states); i++)
					s->key_states[i] &= ~KeyStateJust;

				for (auto i = 0; i < FLAME_ARRAYSIZE(s->mouse_buttons); i++)
					s->mouse_buttons[i] &= ~KeyStateJust;

				s->_priv->mouse_prev_pos = s->mouse_pos;
				s->mouse_scroll = 0;
			}

			_priv->counting_frame++;
			auto et = _priv->last_time;
			_priv->last_time = get_now_ns();
			et = _priv->last_time - et;
			elapsed_time = et / 1000000000.f;
		}
	}

	SurfaceManager *create_surface_manager()
	{
		auto m = new SurfaceManager;
		m->fps = 0;
		m->elapsed_time = 0.f;

		m->_priv = new SurfaceManagerPrivate;

		return m;
	}

	void destroy_surface_manager(SurfaceManager *m)
	{
		for (auto &s : m->_priv->surfaces)
		{
			DestroyWindow(s->_priv->hWnd);
			delete s->_priv;
			delete s;
		}
		delete m->_priv;
		delete m;
	}
}
