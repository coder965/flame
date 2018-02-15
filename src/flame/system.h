#pragma once

#include <string>

namespace tke
{
	void *get_hinst();
	int get_screen_cx();
	int get_screen_cy();
	std::string get_exe_path();

	void exec(const std::string &filename, const std::string &parameters);
}
