#pragma once

#include <functional>
#include <string>

namespace flame
{
	extern bool only_2d;

	extern long long now_ns;
	extern double elapsed_time;

	long long get_now_ns();

	void begin_profile(const std::string &name);
	void end_profile();

	extern unsigned long long total_frame_count;
	extern uint32_t FPS;

	int init(int _resolution_x, int _resolution_y, int debug_level, bool _only_2d);

	void run();
}
