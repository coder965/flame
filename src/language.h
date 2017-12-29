#pragma once

#include <string>

namespace tke
{
	std::string translate(int srcCP, int dstCP, const std::string &src);
	std::string japanese_to_chinese(const std::string &src);
}