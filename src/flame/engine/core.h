#pragma once

#include <functional>

namespace tke
{
	int init(const std::string &_engine_path, int _resolution_x, int _resolution_y, int debug_level, bool _only_2d = false);

	void run();
}
