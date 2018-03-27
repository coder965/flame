#pragma once

#include <functional>
#include <string>
#include <vector>

namespace flame
{
	extern bool only_2d;

	extern long long now_ns;
	extern double elapsed_time;

	long long get_now_ns();

	struct Profile
	{
		std::string name;
		long long time;
	};

	extern std::vector<Profile> profiles;

	extern long long p_total_time;
	extern long long p_head_time;
	extern long long p_ui_begin_time;
	extern long long p_ui_end_time;
	extern long long p_render_time;
	extern long long p_tail_time;

	void begin_profile(const std::string &name);
	void end_profile();

	extern unsigned long long total_frame_count;
	extern uint32_t FPS;

	int init(int _resolution_x, int _resolution_y, int debug_level, bool _only_2d);

	void run();
}
