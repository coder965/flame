#include <Windows.h>

int myProfiler;

static HHOOK hook;

LRESULT CALLBACK f(int code, WPARAM wParam, LPARAM lParam)
{
	MessageBeep(0);
	return CallNextHookEx(hook, code, wParam, lParam);
}

struct Init
{
	Init()
	{
		auto inst = GetModuleHandle(nullptr);

		hook = SetWindowsHookEx(WH_KEYBOARD_LL, f, inst, 0);
	}
};
static Init init;
