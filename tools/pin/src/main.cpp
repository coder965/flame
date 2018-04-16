#include <flame/system.h>

#include <Windows.h>

int main(int argc, char **args)
{
	auto w = FindWindow("Chrome_WidgetWin_1", NULL);
	w = GetParent(w);
	SetForegroundWindow(w);
	auto b = SetWindowPos(w, HWND_TOPMOST, 0, 0, 0, 0,  SWP_NOSIZE);

	return 0;
}
