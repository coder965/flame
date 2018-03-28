#pragma once

extern "C"
{
	__declspec(dllexport) flame::ui::Window *flame_create_ui_window();
	__declspec(dllexport) void flame_destroy_ui_window(flame::ui::Window *w);
}
