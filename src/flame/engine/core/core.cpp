#include <map>
#include <regex>
#include <list>
#include <chrono>
#include <stack>

#include <flame/global.h>
#include <flame/common/system.h>
#include <flame/engine/core/core.h>
#include <flame/engine/core/application.h>
#include <flame/engine/ui/ui.h>
#include <flame/engine/physics/physics.h>
#include <flame/engine/sound/sound.h>
#include <flame/engine/entity/entity.h>

namespace flame
{
	bool only_2d = false;

	long long now_ns;
	double elapsed_time;

	long long get_now_ns()
	{
		return std::chrono::time_point_cast<std::chrono::nanoseconds>(
			std::chrono::system_clock::now()
			).time_since_epoch().count();
	}

	std::stack<Profile> profile_stack;
	std::vector<Profile> profiles;

	long long p_total_time = 1;
	long long p_head_time;
	long long p_ui_begin_time;
	long long p_ui_end_time;
	long long p_render_time;
	long long p_tail_time;

	void begin_profile(const std::string &name)
	{
		Profile p;
		p.name = name;
		p.time = get_now_ns();
		profile_stack.push(p);
	}

	void end_profile()
	{
		profiles.push_back(profile_stack.top());
		profile_stack.pop();
		profiles.back().time = get_now_ns() - profiles.back().time;
	}

	unsigned long long total_frame_count = 0;
	uint32_t FPS;

	int init(int _resolution_x, int _resolution_y, int debug_level, bool _only_2d)
	{
		printf("%d\n", GetCurrentProcessId());

		auto init_start_time = GetTickCount();

		only_2d = _only_2d;
#ifdef _MSVC_LANG
		SetProcessDPIAware();
#endif

		init_graphics(debug_level > 0, _resolution_x, _resolution_y);
		ui::init();
		init_sound();
		if (!only_2d)
		{
#if FLAME_ENABLE_PHYSICS
			init_physics();
#endif
			init_entity();
		}

		printf("\n=====INFO=====\nengine init finished - %d ms\n==============\n", GetTickCount() - init_start_time);

		return NoErr;
	}

	static long long _last_sec_time;

	void run()
	{
		assert(app);

		now_ns = get_now_ns();
		_last_sec_time = now_ns;

		static long long last_time_ns;

		for (;;)
		{
			profiles.clear();

			begin_profile("total");

			begin_profile("head");
			MSG msg;
			while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				if (msg.message == WM_QUIT)
					return;
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}

			last_time_ns = now_ns;
			now_ns = get_now_ns();
			elapsed_time = (now_ns - last_time_ns) / 1000000000.0;

			static unsigned int frame_count = 0;
			frame_count++;
			total_frame_count++;
			bool do_copy_profile = false;
			if (now_ns - _last_sec_time >= 1000000000)
			{
				FPS = std::max(frame_count, 1U);
				_last_sec_time = now_ns;
				frame_count = 0;
				do_copy_profile = true;
			}

			app->update();

			end_profile();

			if (do_copy_profile)
			{
				p_head_time = profiles[0].time;
				p_ui_begin_time = profiles[1].time;
				p_ui_end_time = profiles[2].time;
				p_render_time = profiles[3].time;
				p_tail_time = profiles[4].time;
				p_total_time = profiles[5].time;
			}
		}
	}
}
