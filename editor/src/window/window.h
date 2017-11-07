#pragma once

#include <string>
#include <vector>
#include <memory>

struct Window
{
	std::string name;
	bool opened;

	Window(const std::string &_name);
};

extern std::vector<std::unique_ptr<Window>> windows;
