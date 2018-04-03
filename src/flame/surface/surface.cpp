#include <algorithm>
#include <list>
#include <queue>
#include <assert.h>
#define NOMINMAX
#include <Windows.h>

#include <flame/filesystem.h>
#include <flame/system.h>
#include <flame/image.h>
#include <flame/surface/surface.h>

namespace flame
{
	struct SurfaceManagerImpl
	{
		std::list<Surface*> surfaces;
	};

	enum KeyEventType
	{
		KeyEventNull,
		KeyEventDown,
		KeyEventUp
	};

	struct SurfaceImpl
	{
		int cx;
		int cy;
		int style;
		std::string title;

		HWND hWnd;

		int key_states[512];

		int mouse_x;
		int mouse_y;
		int mouse_prev_x;
		int mouse_prev_y;
		int mouse_disp_x;
		int mouse_disp_y;
		int mouse_scroll;

		int mouse_button[3];

		std::list<std::function<void(Surface *, int)>> keydown_listeners;
		std::list<std::function<void(Surface *, int)>> keyup_listeners;
		std::list<std::function<void(Surface *, int)>> char_listeners;
		std::list<std::function<void(Surface *, int, int)>> resize_listeners;
		std::list<std::function<void(Surface *)>> destroy_listeners;

		KeyEventType key_event_type;
		int key_event_key;

		KeyEventType mouse_event_type;
		int mouse_event_key;
		bool mouse_move_event;
		int mouse_scroll_event;

		std::queue<int> char_events;

		bool resize_event;

		bool destroy_event;

		SurfaceImpl()
		{
			hWnd = 0;

			for (auto i = 0; i < TK_ARRAYSIZE(key_states); i++)
				key_states[i] = KeyStateUp;

			mouse_x = mouse_y = 0;
			mouse_prev_x = mouse_prev_y = 0;
			mouse_disp_x = mouse_disp_y = 0;
			mouse_scroll = 0;

			for (auto i = 0; i < TK_ARRAYSIZE(mouse_button); i++)
				mouse_button[i] = KeyStateUp;

			key_event_type = KeyEventNull;
			key_event_key = 0;

			mouse_event_type = KeyEventNull;
			mouse_event_key = 0;
			mouse_move_event = false;
			mouse_scroll_event = false;

			resize_event = false;

			destroy_event = false;
		}
	};

	static LRESULT CALLBACK _wnd_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		auto pSurface = (Surface*)GetWindowLong(hWnd, 0);

		if (pSurface)
		{
			auto impl = (SurfaceImpl*)pSurface->impl;

			switch (message)
			{
				case WM_KEYDOWN:
					impl->key_states[wParam] = KeyStateDown | KeyStateJust;
					impl->key_event_type = KeyEventDown;
					impl->key_event_key = wParam;
					break;
				case WM_KEYUP:
					impl->key_states[wParam] = KeyStateUp | KeyStateJust;
					impl->key_event_type = KeyEventUp;
					impl->key_event_key = wParam;
					break;
				case WM_CHAR:
					impl->char_events.push(wParam);
					break;
				case WM_LBUTTONDOWN:
					impl->mouse_button[0] = KeyStateDown | KeyStateJust;
					impl->mouse_x = LOWORD(lParam);
					impl->mouse_y = HIWORD(lParam);
					impl->mouse_event_type = KeyEventDown;
					impl->mouse_event_key = 0;
					SetCapture(hWnd);
					break;
				case WM_LBUTTONUP:
					impl->mouse_button[0] = KeyStateUp | KeyStateJust;
					impl->mouse_x = LOWORD(lParam);
					impl->mouse_y = HIWORD(lParam);
					impl->mouse_event_type = KeyEventUp;
					impl->mouse_event_key = 0;
					ReleaseCapture();
					break;
				case WM_MBUTTONDOWN:
					impl->mouse_button[2] = KeyStateDown | KeyStateJust;
					impl->mouse_x = LOWORD(lParam);
					impl->mouse_y = HIWORD(lParam);
					impl->mouse_event_type = KeyEventDown;
					impl->mouse_event_key = 2;
					SetCapture(hWnd);
					break;
				case WM_MBUTTONUP:
					impl->mouse_button[2] = KeyStateUp | KeyStateJust;
					impl->mouse_x = LOWORD(lParam);
					impl->mouse_y = HIWORD(lParam);
					impl->mouse_event_type = KeyEventUp;
					impl->mouse_event_key = 2;
					ReleaseCapture();
					break;
				case WM_RBUTTONDOWN:
					impl->mouse_button[1] = KeyStateDown | KeyStateJust;
					impl->mouse_x = LOWORD(lParam);
					impl->mouse_y = HIWORD(lParam);
					impl->mouse_event_type = KeyEventDown;
					impl->mouse_event_key = 1;
					SetCapture(hWnd);
					break;
				case WM_RBUTTONUP:
					impl->mouse_button[1] = KeyStateUp | KeyStateJust;
					impl->mouse_x = LOWORD(lParam);
					impl->mouse_y = HIWORD(lParam);
					impl->mouse_event_type = KeyEventUp;
					impl->mouse_event_key = 1;
					ReleaseCapture();
					break;
				case WM_MOUSEMOVE:
					impl->mouse_x = LOWORD(lParam);
					impl->mouse_y = HIWORD(lParam);
					impl->mouse_move_event = true;
					break;
				case WM_MOUSEWHEEL:
					impl->mouse_scroll = (short)HIWORD(wParam) > 0 ? 1 : -1;
					impl->mouse_scroll_event = true;
					break;
				case WM_DESTROY:
					impl->destroy_event = true;
				case WM_SIZE:
				{
					auto x = std::max((int)LOWORD(lParam), 1);
					auto y = std::max((int)HIWORD(lParam), 1);
					if (x != impl->cx || y != impl->cy)
					{
						impl->cx = x;
						impl->cy = y;
						impl->resize_event = true;
					}
					break;
				}
			}
		}

		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	SurfaceManager *create_surface_manager()
	{
		auto m = new SurfaceManager;
		m->impl = new SurfaceManagerImpl;
		return m;
	}

	void destroy_surface_manager(SurfaceManager *m)
	{
		delete m->impl;
		delete m;
	}

	int surface_manager_run(SurfaceManager *m, void(*idle_callback)())
	{
		auto impl = (SurfaceManagerImpl*)m->impl;
		if (impl->surfaces.size() == 0)
			return 1;

		assert(idle_callback);

		for (;;)
		{
			MSG msg;
			while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				if (msg.message == WM_QUIT)
					return;
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}

			for (auto it = impl->surfaces.begin(); it != impl->surfaces.end(); )
			{
				auto pSurface = *it;
				auto s_impl = (SurfaceImpl*)pSurface->impl;
				if (s_impl->destroy_event)
				{
					for (auto &e : s_impl->destroy_listeners)
						e(pSurface);
					it = impl->surfaces.erase(it);
				}
			}

			if (impl->surfaces.empty())
				return 0;

			idle_callback();
		}
	}

	Surface *create_surface(SurfaceManager *m, int _cx, int _cy, int _style, const std::string &_title)
	{
		static bool initialized = false;
		if (!initialized)
		{
			WNDCLASSEXA wcex;
			wcex.cbSize = sizeof(WNDCLASSEXA);
			wcex.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
			wcex.lpfnWndProc = _wnd_proc;
			wcex.cbClsExtra = 0;
			wcex.cbWndExtra = sizeof(TK_ULONG_PTR);
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
		s->impl = new SurfaceImpl;
		auto impl = (SurfaceImpl*)s->impl;
		impl->title = _title;

		set_surface_size(s, _cx, _cy, _style);

		SetWindowLongPtr(impl->hWnd, 0, (LONG_PTR)s);

		return s;
	}

	void destroy_surface(SurfaceManager *m, Surface *s)
	{
		auto impl = (SurfaceImpl*)s->impl;
		DestroyWindow(impl->hWnd);
		for (auto &e : impl->destroy_listeners)
			e(s);
		delete impl;
		delete s;
	}

	std::string get_surface_title(Surface *s)
	{

	}

	IVEC2 get_surface_size(Surface *s)
	{

	}

	void set_surface_size(Surface *s, int _cx, int _cy, int _style)
	{
		auto impl = (SurfaceImpl*)s->impl;

		if (_cx > 0)
			impl->cx = _cx;
		if (_cy > 0)
			impl->cy = _cy;
		impl->style = _style;

		auto win32_style = WS_VISIBLE;
		if ((impl->style & SurfaceStyleFrame) && !(impl->style & SurfaceStyleFullscreen))
		{
			RECT rect = { 0, 0, impl->cx, impl->cy };
			AdjustWindowRect(&rect, WS_CAPTION, false);
			_cx = rect.right - rect.left;
			_cy = rect.bottom - rect.top;

			win32_style |= WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;

			if (impl->style & SurfaceStyleResizable)
				win32_style |= WS_THICKFRAME | WS_MAXIMIZEBOX;
		}
		else
		{
			win32_style |= WS_BORDER;
			if (impl->style & SurfaceStyleFullscreen)
			{
				_cx = get_screen_cx();
				_cy = get_screen_cy();
			}
		}

		auto x = (get_screen_cx() - _cx) / 2;
		auto y = (get_screen_cy() - _cy) / 2;

		auto impl = (SurfaceImpl*)s->impl;
		if (impl->hWnd)
		{
			SetWindowLong(impl->hWnd, GWL_STYLE, win32_style);
			SetWindowPos(impl->hWnd, HWND_TOP, x, y, _cx, _cy, SWP_NOZORDER);
		}
		else
		{
			impl->hWnd = CreateWindowA("tke_wnd", impl->title.c_str(), win32_style,
				x, y, _cx, _cy, NULL, NULL, (HINSTANCE)get_hinst(), NULL);
		}
	}

	void set_surface_maximized(Surface *s, bool v)
	{
		auto impl = (SurfaceImpl*)s->impl;
		ShowWindow(impl->hWnd, v ? SW_SHOWMAXIMIZED : SW_SHOWNORMAL);
	}

	void *add_keydown_listener(Surface *s, const std::function<void(Surface *s, int)> &e)
	{
		auto impl = (SurfaceImpl*)s->impl;
		impl->keydown_listeners.push_back(e);
		return &impl->keydown_listeners.back();
	}

	void *add_keyup_listener(Surface *s, const std::function<void(Surface *s, int)> &e)
	{
		auto impl = (SurfaceImpl*)s->impl;
		impl->keydown_listeners.push_back(e);
		return &impl->keydown_listeners.back();
	}

	void *add_char_listener(Surface *s, const std::function<void(Surface *s, int)> &e)
	{
		auto impl = (SurfaceImpl*)s->impl;
		impl->char_listeners.push_back(e);
		return &impl->char_listeners.back();
	}

	void *add_resize_listener(Surface *s, const std::function<void(Surface *s, int, int)> &e)
	{
		auto impl = (SurfaceImpl*)s->impl;
		impl->resize_listeners.push_back(e);
		return &impl->resize_listeners.back();
	}

	void *add_destroy_listener(Surface *s, const std::function<void(Surface *s)> &e)
	{
		auto impl = (SurfaceImpl*)s->impl;
		impl->destroy_listeners.push_back(e);
		return &impl->destroy_listeners.back();
	}

	void remove_keydown_listener(Surface *s, void *p)
	{
		auto impl = (SurfaceImpl*)s->impl;
		for (auto it = impl->keydown_listeners.begin(); it != impl->keydown_listeners.end(); it++)
		{
			if (&(*it) == p)
			{
				impl->keydown_listeners.erase(it);
				return;
			}
		}
	}

	void remove_keyup_listener(Surface *s, void *p)
	{
		auto impl = (SurfaceImpl*)s->impl;
		for (auto it = impl->keyup_listeners.begin(); it != impl->keyup_listeners.end(); it++)
		{
			if (&(*it) == p)
			{
				impl->keyup_listeners.erase(it);
				return;
			}
		}
	}

	void remove_char_listener(Surface *s, void *p)
	{
		auto impl = (SurfaceImpl*)s->impl;
		for (auto it = impl->char_listeners.begin(); it != impl->char_listeners.end(); it++)
		{
			if (&(*it) == p)
			{
				impl->char_listeners.erase(it);
				return;
			}
		}
	}

	void remove_resize_listener(Surface *s, void *p)
	{
		auto impl = (SurfaceImpl*)s->impl;
		for (auto it = impl->keydown_listeners.begin(); it != impl->keydown_listeners.end(); it++)
		{
			if (&(*it) == p)
			{
				impl->keydown_listeners.erase(it);
				return;
			}
		}
	}

	void remove_destroy_listener(Surface *s, void *p)
	{
		auto impl = (SurfaceImpl*)s->impl;
		for (auto it = impl->keydown_listeners.begin(); it != impl->keydown_listeners.end(); it++)
		{
			if (&(*it) == p)
			{
				impl->keydown_listeners.erase(it);
				return;
			}
		}
	}
}
