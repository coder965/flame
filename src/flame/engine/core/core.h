#pragma once

#include <functional>

namespace flame
{
	extern bool only_2d;

	extern std::string engine_path;

	extern long long now_ns;
	extern double elapsed_time;

	long long get_now_time_ns();

	extern unsigned long long total_frame_count;
	extern uint32_t FPS;

	int init(const std::string &_engine_path, int _resolution_x, int _resolution_y, int debug_level, bool watch_shader_file, bool _only_2d);

	void run();
}
