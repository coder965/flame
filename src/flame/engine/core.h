#pragma once

#include <functional>

namespace flame
{
	int init(const std::string &_engine_path, int _resolution_x, int _resolution_y, int debug_level, bool watch_shader_file, bool _only_2d);

	void run();
}
