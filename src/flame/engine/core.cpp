#include <map>
#include <regex>
#include <list>

#include <flame/global.h>
#include <flame/engine/core.h>
#include <flame/engine/system.h>
#include <flame/engine/application.h>
#include <flame/graphics/renderer.h>
#include <flame/ui/ui.h>
#include <flame/physics/physics.h>
#include <flame/sound/sound.h>
#include <flame/entity/model.h>
#include <flame/utils/pick_up.h>

namespace tke
{
	int init(const std::string &_engine_path, int _resolution_x, int _resolution_y, int debug_level, bool _only_2d)
	{
		auto init_start_time = GetTickCount();

		only_2d = _only_2d;

#ifdef _MSVC_LANG
		SetProcessDPIAware();
#endif

		engine_path = _engine_path;
		shader_path = "src/shader/";
		resolution.set(_resolution_x, _resolution_y);

		initVulkan(debug_level > 0);

		ui::init();

		if (!only_2d)
		{
			initModel();
			init_pick_up();

			initPhysics();
		}

		//initSound();
		printf("\n=====INFO=====\nengine init finished - %d ms\n==============\n", GetTickCount() - init_start_time);

		return NoErr;
	}

	static long long _last_sec_time;

	void run()
	{
		assert(app);

		now_ns = get_now_time_ns();
		_last_sec_time = now_ns;

		static long long last_time_ns;

		for (;;)
		{
			MSG msg;
			if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				if (msg.message == WM_QUIT)
					return;
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else
			{
				last_time_ns = now_ns;
				now_ns = get_now_time_ns();
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
}
