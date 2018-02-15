#pragma once

inline constexpr unsigned int _HASH(char const * str, unsigned int seed)
{
	return 0 == *str ? seed : _HASH(str + 1, seed ^ (*str + 0x9e3779b9 + (seed << 6) + (seed >> 2)));
}

#define HASH(x) (_HASH(x, 0))

template <unsigned int N>
struct EnsureConst
{
	static const unsigned int value = N;
};

#define CHASH(x) (EnsureConst<_HASH(x, 0)>::value)

#include <string>

namespace tke
{
	int get_str_line_number(const char *str);

	std::string string_cut(const std::string &str, int length);

	std::string translate(int srcCP, int dstCP, const std::string &src);
	std::string japanese_to_chinese(const std::string &src);
}
