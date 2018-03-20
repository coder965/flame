#include <map>
#include <regex>
#include <list>
#include <chrono>

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

	struct Profile
	{
		std::string name;
		long long time;
	};

	std::vector<Profile> profiles;

	void begin_profile(const std::string &name)
	{
		Profile p;
		p.name = name;
		p.time = get_now_ns();
		profiles.push_back(p);
	}

	void end_profile()
	{
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
			if (now_ns - _last_sec_time >= 1000000000)
			{
				FPS = std::max(frame_count, 1U);
				_last_sec_time = now_ns;
				frame_count = 0;
			}

			app->update();
		}
	}
}
