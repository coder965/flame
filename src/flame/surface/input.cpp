
#include <Windows.h>

#include <flame/global.h>

namespace flame
{

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
