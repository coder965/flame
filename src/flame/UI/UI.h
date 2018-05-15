#pragma once

#ifdef _FLAME_UI_EXPORTS
#define FLAME_UI_EXPORTS __declspec(dllexport)
#else
#define FLAME_UI_EXPORTS __declspec(dllimport)
#endif

namespace flame
{
	namespace graphics
	{
		struct Device;
		struct Commandbuffer;
		struct Renderpass;
		struct Framebuffer;
	}

	namespace UI
	{
		struct ValueInterpolater
		{
			float v;
			float start_value;
			float end_value;
			float curr_time;
			float max_time;

			inline void step(float delta_time)
			{
				curr_time += delta_time;
				if (curr_time >= max_time)
					curr_time = 0.f;
				v = start_value + (end_value - start_value) *
					(curr_time / max_time);
			}
		};
	}
}

