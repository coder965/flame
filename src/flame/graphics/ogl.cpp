#include "ogl.h"
#include "graphics_private.h"

#include <Windows.h>

namespace flame
{
	namespace graphics
	{
#if !defined(FLAME_GRAPHICS_VULKAN)

		LRESULT CALLBACK DummyWndProc(HWND p0, UINT p1, WPARAM p2, LPARAM p3)
		{
			return DefWindowProc(p0, p1, p2, p3);
		}

		void ogl_init()
		{
			HINSTANCE hInstance = GetModuleHandle(NULL);

			WNDCLASS dummy_wc;
			memset(&dummy_wc, 0, sizeof(WNDCLASS));
			dummy_wc.hInstance = hInstance;
			dummy_wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
			dummy_wc.lpfnWndProc = DummyWndProc;
			dummy_wc.lpszClassName = "DummyClass";
			if (!RegisterClass(&dummy_wc))
			{
				assert(0);
				return;
			}

			HWND dummy_hWnd = CreateWindow("DummyClass", "Dummy", WS_OVERLAPPEDWINDOW,
				0, 0, 100, 100, NULL, NULL, hInstance, NULL);
			if (!dummy_hWnd)
			{
				auto error = GetLastError();
				assert(0);
				return;
			}

			PIXELFORMATDESCRIPTOR dummy_pfd = {
				sizeof(PIXELFORMATDESCRIPTOR),
				1,
				PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
				PFD_TYPE_RGBA,
				24,
				0, 0, 0, 0, 0, 0,
				0,
				0,
				0,
				0, 0, 0, 0,
				0,
				0,
				0,
				PFD_MAIN_PLANE,
				0,
				0, 0, 0
			};

			HDC dummy_hDC = GetDC(dummy_hWnd);
			int dummy_pf = ChoosePixelFormat(dummy_hDC, &dummy_pfd);
			SetPixelFormat(dummy_hDC, dummy_pf, &dummy_pfd);

			HGLRC dummy_hRC = wglCreateContext(dummy_hDC);
			HDC last_hDC = wglGetCurrentDC();
			HGLRC last_hRC = wglGetCurrentContext();
			wglMakeCurrent(dummy_hDC, dummy_hRC);

			glewInit();

			wglMakeCurrent(last_hDC, last_hRC);
			wglDeleteContext(dummy_hRC);
			ReleaseDC(dummy_hWnd, dummy_hDC);
			DestroyWindow(dummy_hWnd);
			UnregisterClass("DummyClass", hInstance);
		}

		void ogl_clear()
		{
			//glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);
			glClearColor(0.f, 0.f, 0.f, 1.f);
			glClear(GL_COLOR_BUFFER_BIT);
		}
#endif
	}
}
